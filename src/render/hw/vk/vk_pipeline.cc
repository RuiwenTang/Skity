#include "src/render/hw/vk/vk_pipeline.hpp"

#ifdef SKITY_LOG
#undef SKITY_LOG
#endif

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_font_texture.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

#define SKITY_DEFAULT_BUFFER_SIZE 512

namespace skity {

VKPipeline::VKPipeline(GPUVkContext* ctx)
    : HWPipeline(),
      ctx_(ctx),
      vk_memory_allocator_(VKMemoryAllocator::CreateMemoryAllocator()) {}

VKPipeline::~VKPipeline() = default;

void VKPipeline::Init() {
  if (!VKInterface::GlobalInterface()) {
    VKInterface::InitGlobalInterface(
        ctx_->GetDevice(), (PFN_vkGetDeviceProcAddr)ctx_->proc_loader);
  }
  vk_memory_allocator_->Init(ctx_);
  InitFrameBuffers();
  InitPipelines();
  InitCMDPool();
  InitFence();
  InitSampler();

  empty_font_texture_ = std::make_unique<VKFontTexture>(
      nullptr, vk_memory_allocator_.get(), this, ctx_);

  empty_font_texture_->Init();
  empty_font_texture_->PrepareForDraw();
}

void VKPipeline::Destroy() {
  vk_memory_allocator_->FreeBuffer(vertex_buffer_.get());
  vk_memory_allocator_->FreeBuffer(index_buffer_.get());

  empty_font_texture_->Destroy();

  DestroySampler();
  DestroyFence();
  DestroyCMDPool();
  DestroyPipelines();
  DestroyFrameBuffers();
  vk_memory_allocator_->Destroy(ctx_);
}

void VKPipeline::Bind() {
  LOG_DEBUG("vk_pipeline Bind");
  prev_pipeline_ = nullptr;
  global_push_const_.dirty = true;
  model_matrix_.dirty = true;
  common_fragment_set_.dirty = true;
  color_info_set_.dirty = true;

  used_font_and_set_.clear();
  empty_font_set_ = VK_NULL_HANDLE;

  CurrentFrameBuffer()->FrameBegin(ctx_);

  // view port
  VkViewport view_port{0,
                       0,
                       static_cast<float>(ctx_->GetFrameExtent().width),
                       static_cast<float>(ctx_->GetFrameExtent().height),
                       0.f,
                       1.f};
  VK_CALL(vkCmdSetViewport, ctx_->GetCurrentCMD(), 0, 1, &view_port);
  // scissor
  VkRect2D scissor{{0, 0}, ctx_->GetFrameExtent()};
  VK_CALL(vkCmdSetScissor, ctx_->GetCurrentCMD(), 0, 1, &scissor);

  // update empty_font_set_
  empty_font_set_ = CurrentFrameBuffer()->ObtainUniformBufferSet(
      ctx_, static_color_pipeline_->GetFontSetLayout());

  // update this empty_font_set_ for future usage
  auto image_info = VKUtils::DescriptorImageInfo(
      empty_font_texture_->GetSampler(), empty_font_texture_->GetImageView(),
      empty_font_texture_->GetImageLayout());

  auto write_set = VKUtils::WriteDescriptorSet(
      empty_font_set_, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0,
      &image_info);

  VK_CALL(vkUpdateDescriptorSets, ctx_->GetDevice(), 1, &write_set, 0,
          VK_NULL_HANDLE);
}

void VKPipeline::UnBind() {
  LOG_DEBUG("vk_pipeline UnBind");
  prev_pipeline_ = nullptr;
}

void VKPipeline::SetViewProjectionMatrix(const glm::mat4& mvp) {
  LOG_DEBUG("vk_pipeline set mvp");
  global_push_const_.value.mvp = mvp;
  global_push_const_.dirty = true;
}

void VKPipeline::SetModelMatrix(const glm::mat4& matrix) {
  LOG_DEBUG("vk_pipeline upload transform matrix");
  model_matrix_.value = matrix;
  model_matrix_.dirty = true;
}

void VKPipeline::SetPipelineColorMode(HWPipelineColorMode mode) {
  LOG_DEBUG("vk_pipeline set color mode");
  color_mode_ = mode;
}

void VKPipeline::SetStrokeWidth(float width) {
  LOG_DEBUG("vk_pipeline set stroke width");
  common_fragment_set_.value.info.g = width;
  common_fragment_set_.dirty = true;
}

void VKPipeline::SetUniformColor(const glm::vec4& color) {
  LOG_DEBUG("vk_pipeline set uniform color");
  color_info_set_.value.user_color = color;
  color_info_set_.dirty = true;
}

void VKPipeline::SetGradientBoundInfo(const glm::vec4& info) {
  LOG_DEBUG("vk_pipeline set gradient bounds");
  gradient_info_set_.value.bounds = info;
  gradient_info_set_.dirty = true;
}

void VKPipeline::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {
  LOG_DEBUG("vk_pipeline set gradient color and stop count");
  gradient_info_set_.value.count.x = color_count;
  gradient_info_set_.value.count.y = pos_count;
  gradient_info_set_.dirty = true;
}

void VKPipeline::SetGradientColors(const std::vector<Color4f>& colors) {
  LOG_DEBUG("vk_pipeline set gradient colors");
  std::memcpy(gradient_info_set_.value.colors, colors.data(),
              sizeof(float) * 4 * colors.size());
  gradient_info_set_.dirty = true;
}

void VKPipeline::SetGradientPositions(const std::vector<float>& pos) {
  LOG_DEBUG("vk_pipeline set gradient stops");
  std::memcpy(gradient_info_set_.value.pos, pos.data(),
              sizeof(float) * pos.size());
  gradient_info_set_.dirty = true;
}

void VKPipeline::UploadVertexBuffer(void* data, size_t data_size) {
  LOG_DEBUG("vk_pipeline upload vertex buffer with size: {}", data_size);

  if (!vertex_buffer_ || vertex_buffer_->BufferSize() < data_size) {
    size_t new_size = vertex_buffer_ ? vertex_buffer_->BufferSize() * 2
                                     : SKITY_DEFAULT_BUFFER_SIZE;
    new_size = std::max(new_size, data_size);
    InitVertexBuffer(new_size);
  }

  vk_memory_allocator_->UploadBuffer(vertex_buffer_.get(), data, data_size);

  uint64_t offset = 0;
  VkBuffer buffer = vertex_buffer_->GetBuffer();
  VK_CALL(vkCmdBindVertexBuffers, ctx_->GetCurrentCMD(), 0, 1, &buffer,
          &offset);
}

void VKPipeline::UploadIndexBuffer(void* data, size_t data_size) {
  LOG_DEBUG("vk_pipeline upload index buffer with size: {}", data_size);

  if (!index_buffer_ || index_buffer_->BufferSize() < data_size) {
    size_t new_size = index_buffer_ ? vertex_buffer_->BufferSize() * 2
                                    : SKITY_DEFAULT_BUFFER_SIZE;
    new_size = std::max(new_size, data_size);
    InitIndexBuffer(new_size);
  }

  vk_memory_allocator_->UploadBuffer(index_buffer_.get(), data, data_size);

  VK_CALL(vkCmdBindIndexBuffer, ctx_->GetCurrentCMD(),
          index_buffer_->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void VKPipeline::SetGlobalAlpha(float alpha) {
  LOG_DEBUG("vk_pipeline set global alpha");
  common_fragment_set_.value.info.r = alpha;
  common_fragment_set_.dirty = true;
}

void VKPipeline::EnableStencilTest() {
  LOG_DEBUG("vk_pipeline enable stencil test");
  enable_stencil_test_ = true;
}

void VKPipeline::DisableStencilTest() {
  LOG_DEBUG("vk_pipeline disable stencil test");
  enable_stencil_test_ = false;
}

void VKPipeline::EnableColorOutput() {
  LOG_DEBUG("vk_pipeline enable color output");
  enable_color_output_ = true;
}

void VKPipeline::DisableColorOutput() {
  LOG_DEBUG("vk_pipeline disable color output");
  enable_color_output_ = false;
}

void VKPipeline::UpdateStencilMask(uint8_t write_mask) {
  LOG_DEBUG("vk_pipeline set stencil write mask {:x}", write_mask);
  stencil_write_mask_ = write_mask;
}

void VKPipeline::UpdateStencilOp(HWStencilOp op) {
  LOG_DEBUG("vk_pipeline set stencil op");
  stencil_op_ = op;
}

void VKPipeline::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                   uint32_t compare_mask) {
  LOG_DEBUG("vk_pipeline set stencil func with value : {} ; mask : {:x}", value,
            compare_mask);

  stencil_func_ = func;
  stencil_value_ = value;
  stencil_compare_mask_ = compare_mask;
}

void VKPipeline::DrawIndex(uint32_t start, uint32_t count) {
  LOG_DEBUG("vk_pipeline draw_index [ {} -> {} ]", start, count);

  LOG_DEBUG("color output enable : {}", enable_color_output_);
  LOG_DEBUG("stencil output enable : {}", enable_stencil_test_);
  if (enable_stencil_test_) {
    LOG_DEBUG(
        "stencil func : {}, stencil op: {}, stencil write mask : {:x}, stencil "
        "compare op : {:x}",
        stencil_func_, stencil_op_, stencil_write_mask_, stencil_compare_mask_);
  }
  LOG_DEBUG("color mode = {}", color_mode_);

  VKPipelineWrapper* picked_pipeline = nullptr;
  if (color_mode_ == HWPipelineColorMode::kStencil) {
    picked_pipeline = PickStencilPipeline();
  } else if (color_mode_ == HWPipelineColorMode::kUniformColor) {
    picked_pipeline = PickColorPipeline();
  } else if (color_mode_ == HWPipelineColorMode::kImageTexture) {
    picked_pipeline = PickImagePipeline();
  } else if (color_mode_ == HWPipelineColorMode::kLinearGradient ||
             color_mode_ == HWPipelineColorMode::kRadialGradient) {
    gradient_info_set_.value.count.z = color_mode_;
    picked_pipeline = PickGradientPipeline();
  }

  BindPipelineIfNeed(picked_pipeline);

  // push constant
  UpdatePushConstantIfNeed(picked_pipeline);
  // set 0 for both vertex and fragment shader
  UpdateTransformMatrixIfNeed(picked_pipeline);
  // set 1 for fragment
  UpdateCommonSetIfNeed(picked_pipeline);
  // set 2 color for fragment
  UpdateColorInfoIfNeed(picked_pipeline);
  // set 3 font for fragment
  UpdateFontInfoIfNeed(picked_pipeline);
  // dynamic state to use stencil discard
  UpdateStencilConfigIfNeed(picked_pipeline);

  VK_CALL(vkCmdDrawIndexed, ctx_->GetCurrentCMD(), count, 1, start, 0, 0);
}

void VKPipeline::BindTexture(HWTexture* texture, uint32_t slot) {
  LOG_DEBUG("vk_pipeline bind to {}", slot);
  VKTexture* vk_texture = (VKTexture*)texture;

  vk_texture->PrepareForDraw();
  if (slot == 0) {
    image_texture_ = vk_texture;
  } else {
    font_texture_ = vk_texture;
  }
}

VkCommandBuffer VKPipeline::ObtainInternalCMD() {
  VkCommandBufferAllocateInfo buffer_info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  buffer_info.commandBufferCount = 1;
  buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  buffer_info.commandPool = vk_cmd_pool_;

  VkCommandBuffer cmd;

  if (VK_CALL(vkAllocateCommandBuffers, ctx_->GetDevice(), &buffer_info,
              &cmd) != VK_SUCCESS) {
    LOG_ERROR("Faled allocate internal command buffer!");
    return VK_NULL_HANDLE;
  }

  VkCommandBufferBeginInfo begin_info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CALL(vkBeginCommandBuffer, cmd, &begin_info);

  return cmd;
}

void VKPipeline::SubmitCMD(VkCommandBuffer cmd) {
  VK_CALL(vkEndCommandBuffer, cmd);

  VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  VK_CALL(vkQueueSubmit, ctx_->GetGraphicQueue(), 1, &submit_info, vk_fence_);

  WaitForFence();
  ResetFence();
}

void VKPipeline::WaitForFence() {
  VK_CALL(vkWaitForFences, ctx_->GetDevice(), 1, &vk_fence_, VK_TRUE,
          1000000000);
}

void VKPipeline::ResetFence() {
  VK_CALL(vkResetFences, ctx_->GetDevice(), 1, &vk_fence_);
}

void VKPipeline::InitCMDPool() {
  // create a command pool for internal use
  VkCommandPoolCreateInfo create_info{
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  create_info.queueFamilyIndex = ctx_->GetGraphicQueueIndex();
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (VK_CALL(vkCreateCommandPool, ctx_->GetDevice(), &create_info, nullptr,
              &vk_cmd_pool_) != VK_SUCCESS) {
    LOG_ERROR("Failed create internal command pool!");
  }
}

void VKPipeline::InitFence() {
  VkFenceCreateInfo create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (VK_CALL(vkCreateFence, ctx_->GetDevice(), &create_info, nullptr,
              &vk_fence_) != VK_SUCCESS) {
    LOG_ERROR("Failed create internal fence object!");
  }

  ResetFence();
}

void VKPipeline::InitSampler() {
  // create sampler
  auto sampler_create_info = VKUtils::SamplerCreateInfo();

  VK_CALL(vkCreateSampler, ctx_->GetDevice(), &sampler_create_info, nullptr,
          &vk_sampler_);
}

void VKPipeline::InitFrameBuffers() {
  frame_buffer_.resize(ctx_->GetSwapchainBufferCount());
  for (size_t i = 0; i < frame_buffer_.size(); i++) {
    frame_buffer_[i] =
        std::make_unique<VKFrameBuffer>(vk_memory_allocator_.get());
    frame_buffer_[i]->Init(ctx_);
  }
}

void VKPipeline::DestroyCMDPool() {
  VK_CALL(vkResetCommandPool, ctx_->GetDevice(), vk_cmd_pool_, 0);
  VK_CALL(vkDestroyCommandPool, ctx_->GetDevice(), vk_cmd_pool_, nullptr);
}

void VKPipeline::DestroyFence() {
  VK_CALL(vkDestroyFence, ctx_->GetDevice(), vk_fence_, nullptr);
}

void VKPipeline::DestroySampler() {
  VK_CALL(vkDestroySampler, ctx_->GetDevice(), vk_sampler_, nullptr);
}

void VKPipeline::DestroyPipelines() {
  static_color_pipeline_->Destroy(ctx_);
  stencil_color_pipeline_->Destroy(ctx_);
  stencil_clip_color_pipeline_->Destroy(ctx_);
  stencil_keep_color_pipeline_->Destroy(ctx_);
  static_gradient_pipeline_->Destroy(ctx_);
  stencil_gradient_pipeline_->Destroy(ctx_);
  stencil_clip_gradient_pipeline_->Destroy(ctx_);
  stencil_keep_gradient_pipeline_->Destroy(ctx_);
  static_image_pipeline_->Destroy(ctx_);
  stencil_image_pipeline_->Destroy(ctx_);
  stencil_clip_image_pipeline_->Destroy(ctx_);
  stencil_keep_image_pipeline_->Destroy(ctx_);
  stencil_front_pipeline_->Destroy(ctx_);
  stencil_clip_front_pipeline_->Destroy(ctx_);
  stencil_back_pipeline_->Destroy(ctx_);
  stencil_clip_back_pipeline_->Destroy(ctx_);
  stencil_clip_pipeline_->Destroy(ctx_);
  stencil_replace_pipeline_->Destroy(ctx_);
}

void VKPipeline::DestroyFrameBuffers() {
  for (size_t i = 0; i < frame_buffer_.size(); i++) {
    frame_buffer_[i]->Destroy(ctx_);
  }
}

void VKPipeline::InitPipelines() {
  static_color_pipeline_ = VKPipelineWrapper::CreateStaticColorPipeline(ctx_);
  stencil_color_pipeline_ = VKPipelineWrapper::CreateStencilColorPipeline(ctx_);
  stencil_clip_color_pipeline_ =
      VKPipelineWrapper::CreateStencilClipColorPipeline(ctx_);
  stencil_keep_color_pipeline_ =
      VKPipelineWrapper::CreateStencilKeepColorPipeline(ctx_);
  static_gradient_pipeline_ =
      VKPipelineWrapper::CreateStaticGradientPipeline(ctx_);
  stencil_gradient_pipeline_ =
      VKPipelineWrapper::CreateStencilDiscardGradientPipeline(ctx_);
  stencil_clip_gradient_pipeline_ =
      VKPipelineWrapper::CreateStencilClipGradientPipeline(ctx_);
  stencil_keep_gradient_pipeline_ =
      VKPipelineWrapper::CreateStencilKeepGradientPipeline(ctx_);
  static_image_pipeline_ = VKPipelineWrapper::CreateStaticImagePipeline(ctx_);
  stencil_image_pipeline_ =
      VKPipelineWrapper::CreateStencilDiscardGradientPipeline(ctx_);
  stencil_clip_image_pipeline_ =
      VKPipelineWrapper::CreateStencilClipImagePipeline(ctx_);
  stencil_keep_image_pipeline_ =
      VKPipelineWrapper::CreateStencilKeepImagePipeline(ctx_);
  stencil_front_pipeline_ = VKPipelineWrapper::CreateStencilFrontPipeline(ctx_);
  stencil_clip_front_pipeline_ =
      VKPipelineWrapper::CreateStencilClipFrontPipeline(ctx_);
  stencil_back_pipeline_ = VKPipelineWrapper::CreateStencilBackPipeline(ctx_);
  stencil_clip_back_pipeline_ =
      VKPipelineWrapper::CreateStencilClipBackPipeline(ctx_);
  stencil_clip_pipeline_ = VKPipelineWrapper::CreateStencilClipPipeline(ctx_);
  stencil_replace_pipeline_ =
      VKPipelineWrapper::CreateStencilReplacePipeline(ctx_);
}

void VKPipeline::InitVertexBuffer(size_t new_size) {
  if (vertex_buffer_) {
    vk_memory_allocator_->FreeBuffer(vertex_buffer_.get());
  }
  vertex_buffer_.reset(vk_memory_allocator_->AllocateVertexBuffer(new_size));
}

void VKPipeline::InitIndexBuffer(size_t new_size) {
  if (index_buffer_) {
    vk_memory_allocator_->FreeBuffer(index_buffer_.get());
  }
  index_buffer_.reset(vk_memory_allocator_->AllocateIndexBuffer(new_size));
}

VKPipelineWrapper* VKPipeline::PickColorPipeline() {
  if (!enable_stencil_test_) {
    return static_color_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::NOT_EQUAL) {
    return stencil_color_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::LESS) {
    return stencil_clip_color_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::EQUAL) {
    return stencil_keep_color_pipeline_.get();
  }

  return nullptr;
}

VKPipelineWrapper* VKPipeline::PickStencilPipeline() {
  if (stencil_op_ == HWStencilOp::INCR_WRAP) {
    if (stencil_func_ == HWStencilFunc::ALWAYS) {
      return stencil_front_pipeline_.get();
    } else {
      return stencil_clip_front_pipeline_.get();
    }
  } else if (stencil_op_ == HWStencilOp::DECR_WRAP) {
    if (stencil_func_ == HWStencilFunc::ALWAYS) {
      return stencil_back_pipeline_.get();
    } else {
      return stencil_clip_back_pipeline_.get();
    }
  } else if (stencil_op_ == HWStencilOp::REPLACE) {
    if (stencil_func_ == HWStencilFunc::ALWAYS) {
      return stencil_replace_pipeline_.get();
    } else if (stencil_func_ == HWStencilFunc::NOT_EQUAL) {
      return stencil_clip_pipeline_.get();
    }
  }

  return nullptr;
}

VKPipelineWrapper* VKPipeline::PickGradientPipeline() {
  if (!enable_stencil_test_) {
    return static_gradient_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::NOT_EQUAL) {
    return stencil_gradient_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::LESS) {
    return stencil_clip_gradient_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::EQUAL) {
    return stencil_keep_gradient_pipeline_.get();
  }

  return nullptr;
}

VKPipelineWrapper* VKPipeline::PickImagePipeline() {
  if (!enable_stencil_test_) {
    return static_image_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::NOT_EQUAL) {
    return stencil_image_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::LESS) {
    return stencil_clip_image_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::EQUAL) {
    return stencil_keep_image_pipeline_.get();
  }

  return nullptr;
}

void VKPipeline::BindPipelineIfNeed(VKPipelineWrapper* pipeline) {
  if (pipeline == prev_pipeline_) {
    // no need to call bind pipeline
    return;
  }

  pipeline->Bind(ctx_->GetCurrentCMD());
  prev_pipeline_ = pipeline;
}

void VKPipeline::UpdatePushConstantIfNeed(VKPipelineWrapper* pipeline) {
  if (!global_push_const_.dirty) {
    return;
  }

  global_push_const_.dirty = false;

  pipeline->UploadPushConstant(global_push_const_.value, ctx_->GetCurrentCMD());
}

void VKPipeline::UpdateTransformMatrixIfNeed(VKPipelineWrapper* pipeline) {
// Fixme to solve Uniform set not working
// it seems MoltenVK DescriptorSet has bug
#ifndef __APPLE__
  if (!model_matrix_.dirty) {
    return;
  }
  model_matrix_.dirty = false;
#endif
  pipeline->UploadTransformMatrix(model_matrix_.value, ctx_,
                                  CurrentFrameBuffer(),
                                  vk_memory_allocator_.get());
}

void VKPipeline::UpdateCommonSetIfNeed(VKPipelineWrapper* pipeline) {
// Fixme to solve Uniform set not working
// it seems MoltenVK DescriptorSet has bug
#ifndef __APPLE__
  if (!common_fragment_set_.dirty) {
    return;
  }
  common_fragment_set_.dirty = false;
#endif

  pipeline->UploadCommonSet(common_fragment_set_.value, ctx_,
                            CurrentFrameBuffer(), vk_memory_allocator_.get());
}

void VKPipeline::UpdateStencilConfigIfNeed(VKPipelineWrapper* pipeline) {
  pipeline->UpdateStencilInfo(stencil_value_, ctx_);
}

void VKPipeline::UpdateColorInfoIfNeed(VKPipelineWrapper* pipeline) {
  if (color_info_set_.dirty && pipeline->HasColorSet()) {
    pipeline->UploadUniformColor(color_info_set_.value, ctx_,
                                 CurrentFrameBuffer(),
                                 vk_memory_allocator_.get());
    color_info_set_.dirty = false;
  }

  if (gradient_info_set_.dirty && pipeline->HasColorSet()) {
    pipeline->UploadGradientInfo(gradient_info_set_.value, ctx_,
                                 CurrentFrameBuffer(),
                                 vk_memory_allocator_.get());
    gradient_info_set_.dirty = false;
  }

  if (image_texture_) {
    pipeline->UploadImageTexture(image_texture_, ctx_, CurrentFrameBuffer(),
                                 vk_memory_allocator_.get());
    image_texture_ = nullptr;
  }
  // TODO implement font info update
}

void VKPipeline::UpdateFontInfoIfNeed(VKPipelineWrapper* pipeline) {
  if (font_texture_) {
    auto it = used_font_and_set_.find(font_texture_);
    if (it != used_font_and_set_.end()) {
      if (font_texture_->GetImageLayout() !=
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        assert(false);
      }
      pipeline->UploadFontSet(it->second, ctx_);
    } else {
      auto font_set = CurrentFrameBuffer()->ObtainUniformBufferSet(
          ctx_, pipeline->GetFontSetLayout());

      // update this empty_font_set_ for future usage
      auto image_info = VKUtils::DescriptorImageInfo(
          font_texture_->GetSampler(), font_texture_->GetImageView(),
          font_texture_->GetImageLayout());

      auto write_set = VKUtils::WriteDescriptorSet(
          font_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &image_info);

      VK_CALL(vkUpdateDescriptorSets, ctx_->GetDevice(), 1, &write_set, 0,
              VK_NULL_HANDLE);

      pipeline->UploadFontSet(font_set, ctx_);

      used_font_and_set_.insert(std::make_pair(font_texture_, font_set));
    }

    font_texture_ = nullptr;
  } else {
    pipeline->UploadFontSet(empty_font_set_, ctx_);
  }
}

VKFrameBuffer* VKPipeline::CurrentFrameBuffer() {
  return frame_buffer_[ctx_->GetCurrentBufferIndex()].get();
}

}  // namespace skity