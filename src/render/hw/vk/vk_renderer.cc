#include "src/render/hw/vk/vk_renderer.hpp"

#ifdef SKITY_LOG
#undef SKITY_LOG
#endif

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_font_texture.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_render_target.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

#define SKITY_DEFAULT_BUFFER_SIZE 512

namespace skity {

VkRenderer::VkRenderer(GPUVkContext* ctx, bool use_gs)
    : HWRenderer(), ctx_(ctx), use_gs_(use_gs), vk_memory_allocator_() {}

VkRenderer::~VkRenderer() { delete vk_interface_; }

void VkRenderer::Init() {
  vk_interface_ = VKInterface::InitInterface(
      ctx_->GetDevice(), (PFN_vkGetDeviceProcAddr)ctx_->proc_loader);
  vk_memory_allocator_ =
      VKMemoryAllocator::CreateMemoryAllocator(vk_interface_);
  vk_memory_allocator_->Init(ctx_);
  InitOffScreenRenderPass();
  InitFrameBuffers();
  InitPipelines();
  InitCMDPool();
  InitFence();
  InitSampler();

  empty_font_texture_ = std::make_unique<VKFontTexture>(
      GetInterface(), nullptr, vk_memory_allocator_.get(), this, ctx_);

  empty_font_texture_->Init();
  empty_font_texture_->PrepareForDraw();
}

void VkRenderer::Destroy() {
  vk_memory_allocator_->FreeBuffer(vertex_buffer_.get());
  vk_memory_allocator_->FreeBuffer(index_buffer_.get());

  empty_font_texture_->Destroy();

  DestroySampler();
  DestroyFence();
  DestroyCMDPool();
  DestroyPipelines();
  DestroyFrameBuffers();

  VK_CALL(vkDestroyRenderPass, ctx_->GetDevice(), os_render_pass_, nullptr);
  vk_memory_allocator_->Destroy(ctx_);
}

void VkRenderer::Bind() {
  LOG_DEBUG("vk_pipeline Bind");
  prev_pipeline_ = nullptr;
  global_push_const_.dirty = true;
  model_matrix_.dirty = true;
  common_fragment_set_.dirty = true;
  color_info_set_.dirty = true;

  used_font_and_set_.clear();
  empty_font_set_ = VK_NULL_HANDLE;

  CurrentFrameBuffer()->FrameBegin(ctx_);

  // reset previouse frame allocate command buffer
  VK_CALL(vkResetCommandPool, ctx_->GetDevice(), vk_cmd_pool_, 0);

  // view port
  VkViewport view_port{0,
                       0,
                       static_cast<float>(ctx_->GetFrameExtent().width),
                       static_cast<float>(ctx_->GetFrameExtent().height),
                       0.f,
                       1.f};
  VK_CALL(vkCmdSetViewport, GetCurrentCMD(), 0, 1, &view_port);
  // scissor
  VkRect2D scissor{{0, 0}, ctx_->GetFrameExtent()};
  VK_CALL(vkCmdSetScissor, GetCurrentCMD(), 0, 1, &scissor);

  // update empty_font_set_
  empty_font_set_ = CurrentFrameBuffer()->ObtainUniformBufferSet(
      ctx_,
      color_pipeline_family_->ChoosePipeline(false, false)->GetFontSetLayout());

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

void VkRenderer::UnBind() {
  LOG_DEBUG("vk_pipeline UnBind");
  prev_pipeline_ = nullptr;
}

void VkRenderer::SetViewProjectionMatrix(const glm::mat4& mvp) {
  HWRenderer::SetViewProjectionMatrix(mvp);
  LOG_DEBUG("vk_pipeline set mvp");
  global_push_const_.value.mvp = mvp;
  global_push_const_.dirty = true;
}

void VkRenderer::SetModelMatrix(const glm::mat4& matrix) {
  HWRenderer::SetModelMatrix(matrix);
  LOG_DEBUG("vk_pipeline upload transform matrix");
  model_matrix_.value = matrix;
  model_matrix_.dirty = true;
}

void VkRenderer::SetPipelineColorMode(HWPipelineColorMode mode) {
  LOG_DEBUG("vk_pipeline set color mode");
  color_mode_ = mode;
}

void VkRenderer::SetStrokeWidth(float width) {
  LOG_DEBUG("vk_pipeline set stroke width");
  common_fragment_set_.value.info.g = width;
  common_fragment_set_.dirty = true;
}

void VkRenderer::SetUniformColor(const glm::vec4& color) {
  LOG_DEBUG("vk_pipeline set uniform color");
  color_info_set_.value.user_color = color;
  color_info_set_.dirty = true;
}

void VkRenderer::SetGradientBoundInfo(const glm::vec4& info) {
  LOG_DEBUG("vk_pipeline set gradient bounds");
  gradient_info_set_.value.bounds = info;
  gradient_info_set_.dirty = true;
}

void VkRenderer::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {
  LOG_DEBUG("vk_pipeline set gradient color and stop count");
  gradient_info_set_.value.count.x = color_count;
  gradient_info_set_.value.count.y = pos_count;
  gradient_info_set_.dirty = true;
}

void VkRenderer::SetGradientColors(const std::vector<Color4f>& colors) {
  LOG_DEBUG("vk_pipeline set gradient colors");
  std::memcpy(gradient_info_set_.value.colors, colors.data(),
              sizeof(float) * 4 * colors.size());
  gradient_info_set_.dirty = true;
}

void VkRenderer::SetGradientPositions(const std::vector<float>& pos) {
  LOG_DEBUG("vk_pipeline set gradient stops");
  std::memcpy(gradient_info_set_.value.pos, pos.data(),
              sizeof(float) * pos.size());
  gradient_info_set_.dirty = true;
}

void VkRenderer::UploadVertexBuffer(void* data, size_t data_size) {
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
  VK_CALL(vkCmdBindVertexBuffers, GetCurrentCMD(), 0, 1, &buffer, &offset);
}

void VkRenderer::UploadIndexBuffer(void* data, size_t data_size) {
  LOG_DEBUG("vk_pipeline upload index buffer with size: {}", data_size);

  if (!index_buffer_ || index_buffer_->BufferSize() < data_size) {
    size_t new_size = index_buffer_ ? vertex_buffer_->BufferSize() * 2
                                    : SKITY_DEFAULT_BUFFER_SIZE;
    new_size = std::max(new_size, data_size);
    InitIndexBuffer(new_size);
  }

  vk_memory_allocator_->UploadBuffer(index_buffer_.get(), data, data_size);

  VK_CALL(vkCmdBindIndexBuffer, GetCurrentCMD(), index_buffer_->GetBuffer(), 0,
          VK_INDEX_TYPE_UINT32);
}

void VkRenderer::SetGlobalAlpha(float alpha) {
  LOG_DEBUG("vk_pipeline set global alpha");
  common_fragment_set_.value.info.r = alpha;
  common_fragment_set_.dirty = true;
}

void VkRenderer::EnableStencilTest() {
  LOG_DEBUG("vk_pipeline enable stencil test");
  enable_stencil_test_ = true;
}

void VkRenderer::DisableStencilTest() {
  LOG_DEBUG("vk_pipeline disable stencil test");
  enable_stencil_test_ = false;
}

void VkRenderer::EnableColorOutput() {
  LOG_DEBUG("vk_pipeline enable color output");
  enable_color_output_ = true;
}

void VkRenderer::DisableColorOutput() {
  LOG_DEBUG("vk_pipeline disable color output");
  enable_color_output_ = false;
}

void VkRenderer::UpdateStencilMask(uint8_t write_mask) {
  LOG_DEBUG("vk_pipeline set stencil write mask {:x}", write_mask);
  stencil_write_mask_ = write_mask;
}

void VkRenderer::UpdateStencilOp(HWStencilOp op) {
  LOG_DEBUG("vk_pipeline set stencil op");
  stencil_op_ = op;
}

void VkRenderer::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                   uint32_t compare_mask) {
  LOG_DEBUG("vk_pipeline set stencil func with value : {} ; mask : {:x}", value,
            compare_mask);

  stencil_func_ = func;
  stencil_value_ = value;
  stencil_compare_mask_ = compare_mask;
}

void VkRenderer::DrawIndex(uint32_t start, uint32_t count) {
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

  AbsPipelineWrapper* picked_pipeline = nullptr;
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
  } else if (color_mode_ >= HWPipelineColorMode::kFBOTexture &&
             color_mode_ <= HWPipelineColorMode::kInnerBlurMix) {
    // TODO pick stencil keep or stencil clip pipeline
    picked_pipeline = PickBlurPipeline();
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

  if (picked_pipeline->IsComputePipeline()) {
    picked_pipeline->Dispatch(GetCurrentCMD(), ctx_);
  } else {
    VK_CALL(vkCmdDrawIndexed, GetCurrentCMD(), count, 1, start, 0, 0);
  }
}

void VkRenderer::BindTexture(HWTexture* texture, uint32_t slot) {
  LOG_DEBUG("vk_pipeline bind to {}", slot);
  VKTexture* vk_texture = (VKTexture*)texture;

  vk_texture->PrepareForDraw();
  if (slot == 0) {
    image_texture_ = vk_texture;
  } else {
    font_texture_ = vk_texture;
  }
}

void VkRenderer::BindRenderTarget(HWRenderTarget* render_target) {
  current_target_ = (VKRenderTarget*)render_target;
  // create internal vulkan cmd
  current_target_->StartDraw();

  uint64_t offset = 0;
  VkBuffer buffer = vertex_buffer_->GetBuffer();
  VK_CALL(vkCmdBindVertexBuffers, GetCurrentCMD(), 0, 1, &buffer, &offset);

  VK_CALL(vkCmdBindIndexBuffer, GetCurrentCMD(), index_buffer_->GetBuffer(), 0,
          VK_INDEX_TYPE_UINT32);

  ResetUniformDirty();
}

void VkRenderer::UnBindRenderTarget(HWRenderTarget* render_target) {
  // submit internal vulkan cmd
  current_target_->EndDraw();
  current_target_ = nullptr;
  prev_pipeline_ = nullptr;
  ResetUniformDirty();
}

VkCommandBuffer VkRenderer::ObtainInternalCMD() {
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

void VkRenderer::SubmitCMD(VkCommandBuffer cmd) {
  VK_CALL(vkEndCommandBuffer, cmd);

  VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  VK_CALL(vkQueueSubmit, ctx_->GetGraphicQueue(), 1, &submit_info, vk_fence_);

  WaitForFence();
  ResetFence();
}

void VkRenderer::WaitForFence() {
  VK_CALL(vkWaitForFences, ctx_->GetDevice(), 1, &vk_fence_, VK_TRUE,
          1000000000);
}

void VkRenderer::ResetFence() {
  VK_CALL(vkResetFences, ctx_->GetDevice(), 1, &vk_fence_);
}

void VkRenderer::InitCMDPool() {
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

void VkRenderer::InitFence() {
  VkFenceCreateInfo create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (VK_CALL(vkCreateFence, ctx_->GetDevice(), &create_info, nullptr,
              &vk_fence_) != VK_SUCCESS) {
    LOG_ERROR("Failed create internal fence object!");
  }

  ResetFence();
}

void VkRenderer::InitSampler() {
  // create sampler
  auto sampler_create_info = VKUtils::SamplerCreateInfo();

  VK_CALL(vkCreateSampler, ctx_->GetDevice(), &sampler_create_info, nullptr,
          &vk_sampler_);
}

void VkRenderer::InitFrameBuffers() {
  frame_buffer_.resize(ctx_->GetSwapchainBufferCount());
  for (size_t i = 0; i < frame_buffer_.size(); i++) {
    frame_buffer_[i] = std::make_unique<SKVkFrameBufferData>(
        GetInterface(), vk_memory_allocator_.get());
    frame_buffer_[i]->Init(ctx_);
  }
}

void VkRenderer::DestroyCMDPool() {
  VK_CALL(vkResetCommandPool, ctx_->GetDevice(), vk_cmd_pool_, 0);
  VK_CALL(vkDestroyCommandPool, ctx_->GetDevice(), vk_cmd_pool_, nullptr);
}

void VkRenderer::DestroyFence() {
  VK_CALL(vkDestroyFence, ctx_->GetDevice(), vk_fence_, nullptr);
}

void VkRenderer::DestroySampler() {
  VK_CALL(vkDestroySampler, ctx_->GetDevice(), vk_sampler_, nullptr);
}

void VkRenderer::DestroyPipelines() {
  color_pipeline_family_->Destroy(ctx_);
  gradient_pipeline_family_->Destroy(ctx_);
  image_pipeline_family_->Destroy(ctx_);

  stencil_front_pipeline_->Destroy(ctx_);
  stencil_clip_front_pipeline_->Destroy(ctx_);
  stencil_back_pipeline_->Destroy(ctx_);
  stencil_clip_back_pipeline_->Destroy(ctx_);
  stencil_rec_clip_back_pipeline_->Destroy(ctx_);
  stencil_clip_pipeline_->Destroy(ctx_);
  stencil_rec_clip_pipeline_->Destroy(ctx_);
  stencil_replace_pipeline_->Destroy(ctx_);
  os_stencil_front_pipeline_->Destroy(ctx_);
  os_stencil_back_pipeline_->Destroy(ctx_);
  compute_blur_pipeline_->Destroy(ctx_);
  static_blur_pipeline_->Destroy(ctx_);
  os_static_blur_pipeline_->Destroy(ctx_);
}

void VkRenderer::DestroyFrameBuffers() {
  for (size_t i = 0; i < frame_buffer_.size(); i++) {
    frame_buffer_[i]->Destroy(ctx_);
  }
}

VkCommandBuffer VkRenderer::GetCurrentCMD() {
  if (current_target_) {
    return current_target_->CurrentCMD();
  } else {
    return ctx_->GetCurrentCMD();
  }
}

void VkRenderer::InitPipelines() {
  color_pipeline_family_ = PipelineFamily::CreateColorPipelineFamily();
  color_pipeline_family_->SetInterface(GetInterface());
  color_pipeline_family_->Init(ctx_, use_gs_, os_render_pass_);

  gradient_pipeline_family_ = PipelineFamily::CreateGradientPipelineFamily();
  gradient_pipeline_family_->SetInterface(GetInterface());
  gradient_pipeline_family_->Init(ctx_, use_gs_, os_render_pass_);

  image_pipeline_family_ = PipelineFamily::CreateImagePipelineFamily();
  image_pipeline_family_->SetInterface(GetInterface());
  image_pipeline_family_->Init(ctx_, use_gs_, os_render_pass_);

  stencil_front_pipeline_ = AbsPipelineWrapper::CreateStencilFrontPipeline(
      GetInterface(), ctx_, use_gs_);
  stencil_clip_front_pipeline_ =
      AbsPipelineWrapper::CreateStencilClipFrontPipeline(GetInterface(), ctx_,
                                                         use_gs_);
  stencil_back_pipeline_ = AbsPipelineWrapper::CreateStencilBackPipeline(
      GetInterface(), ctx_, use_gs_);
  stencil_clip_back_pipeline_ =
      AbsPipelineWrapper::CreateStencilClipBackPipeline(GetInterface(), ctx_,
                                                        use_gs_);
  stencil_rec_clip_back_pipeline_ =
      AbsPipelineWrapper::CreateStencilRecClipBackPipeline(GetInterface(), ctx_,
                                                           use_gs_);
  stencil_clip_pipeline_ = AbsPipelineWrapper::CreateStencilClipPipeline(
      GetInterface(), ctx_, use_gs_);
  stencil_rec_clip_pipeline_ = AbsPipelineWrapper::CreateStencilRecClipPipeline(
      GetInterface(), ctx_, use_gs_);
  stencil_replace_pipeline_ = AbsPipelineWrapper::CreateStencilReplacePipeline(
      GetInterface(), ctx_, use_gs_);

  // off screen pipelines
  os_stencil_front_pipeline_ = AbsPipelineWrapper::CreateStencilFrontPipeline(
      GetInterface(), ctx_, use_gs_, os_render_pass_);
  os_stencil_back_pipeline_ = AbsPipelineWrapper::CreateStencilBackPipeline(
      GetInterface(), ctx_, use_gs_, os_render_pass_);

  // effect pipelines
  static_blur_pipeline_ = AbsPipelineWrapper::CreateStaticBlurPipeline(
      GetInterface(), ctx_, use_gs_);
  os_static_blur_pipeline_ = AbsPipelineWrapper::CreateStaticBlurPipeline(
      GetInterface(), ctx_, use_gs_, os_render_pass_);
  compute_blur_pipeline_ =
      AbsPipelineWrapper::CreateComputeBlurPipeline(GetInterface(), ctx_);
}

void VkRenderer::InitVertexBuffer(size_t new_size) {
  if (vertex_buffer_) {
    vk_memory_allocator_->FreeBuffer(vertex_buffer_.get());
  }
  vertex_buffer_.reset(vk_memory_allocator_->AllocateVertexBuffer(new_size));
}

void VkRenderer::InitIndexBuffer(size_t new_size) {
  if (index_buffer_) {
    vk_memory_allocator_->FreeBuffer(index_buffer_.get());
  }
  index_buffer_.reset(vk_memory_allocator_->AllocateIndexBuffer(new_size));
}

void VkRenderer::InitOffScreenRenderPass() {
  std::array<VkAttachmentDescription, 2> attachments = {};

  // color attachment
  attachments[0].format = os_color_format_;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

  // depth stencil attachment
  attachments[1].format = ctx_->GetDepthStencilFormat();
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_ref;
  color_ref.attachment = 0;
  color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference stencil_ref;
  stencil_ref.attachment = 1;
  stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass_desc{};
  subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass_desc.colorAttachmentCount = 1;
  subpass_desc.pColorAttachments = &color_ref;
  subpass_desc.pDepthStencilAttachment = &stencil_ref;

  // Use subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 2> dependencies;

  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo create_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  create_info.attachmentCount = 2;
  create_info.pAttachments = attachments.data();
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass_desc;
  // create_info.dependencyCount = 2;
  // create_info.pDependencies = dependencies.data();

  if (VK_CALL(vkCreateRenderPass, ctx_->GetDevice(), &create_info, nullptr,
              &os_render_pass_) != VK_SUCCESS) {
    LOG_ERROR("VkRenderTarget can not create render pass!");
  }
}

AbsPipelineWrapper* VkRenderer::PickColorPipeline() {
  color_pipeline_family_->UpdateStencilFunc(stencil_func_);

  return color_pipeline_family_->ChoosePipeline(enable_stencil_test_,
                                                current_target_ != nullptr);
}

AbsPipelineWrapper* VkRenderer::PickStencilPipeline() {
  // pick off screen pipeline
  if (current_target_) {
    return PickOSStencilPipeline();
  }

  // pick normal pipeline
  if (stencil_op_ == HWStencilOp::INCR_WRAP) {
    if (stencil_func_ == HWStencilFunc::ALWAYS) {
      return stencil_front_pipeline_.get();
    } else {
      return stencil_clip_front_pipeline_.get();
    }
  } else if (stencil_op_ == HWStencilOp::DECR_WRAP) {
    return PickBackStencilPipeline();
  } else if (stencil_op_ == HWStencilOp::REPLACE) {
    return PickReplaceStencilPipeline();
  }

  return nullptr;
}

AbsPipelineWrapper* VkRenderer::PickOSStencilPipeline() {
  if (stencil_op_ == HWStencilOp::INCR_WRAP) {
    return os_stencil_front_pipeline_.get();
  } else if (stencil_op_ == HWStencilOp::DECR_WRAP) {
    return os_stencil_back_pipeline_.get();
  }

  return nullptr;
}

AbsPipelineWrapper* VkRenderer::PickBackStencilPipeline() {
  if (stencil_func_ == HWStencilFunc::ALWAYS) {
    return stencil_back_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::LESS_OR_EQUAL) {
    return stencil_clip_back_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::EQUAL) {
    return stencil_rec_clip_back_pipeline_.get();
  }

  return nullptr;
}

AbsPipelineWrapper* VkRenderer::PickReplaceStencilPipeline() {
  if (stencil_func_ == HWStencilFunc::ALWAYS) {
    return stencil_replace_pipeline_.get();
  } else if (stencil_func_ == HWStencilFunc::NOT_EQUAL) {
    if (stencil_write_mask_ == 0xFF) {
      // this is a normal clip stencil replace
      return stencil_clip_pipeline_.get();
    } else if (stencil_write_mask_ == 0x0F) {
      // this is recursive clip stencil replace
      return stencil_rec_clip_pipeline_.get();
    }
  }

  return nullptr;
}

AbsPipelineWrapper* VkRenderer::PickGradientPipeline() {
  gradient_pipeline_family_->UpdateStencilFunc(stencil_func_);

  return gradient_pipeline_family_->ChoosePipeline(enable_stencil_test_,
                                                   current_target_ != nullptr);
}

AbsPipelineWrapper* VkRenderer::PickImagePipeline() {
  image_pipeline_family_->UpdateStencilFunc(stencil_func_);

  return image_pipeline_family_->ChoosePipeline(enable_stencil_test_,
                                                current_target_ != nullptr);
}

AbsPipelineWrapper* VkRenderer::PickBlurPipeline() {
  if (current_target_) {
    if (color_mode_ == HWPipelineColorMode::kHorizontalBlur ||
        color_mode_ == HWPipelineColorMode::kVerticalBlur) {
      return compute_blur_pipeline_.get();
    }
    return os_static_blur_pipeline_.get();
  }

  return static_blur_pipeline_.get();
}

void VkRenderer::BindPipelineIfNeed(AbsPipelineWrapper* pipeline) {
  if (pipeline == prev_pipeline_ && current_target_ == nullptr) {
    // no need to call bind pipeline
    return;
  }

  pipeline->Bind(GetCurrentCMD());
  prev_pipeline_ = pipeline;
}

void VkRenderer::UpdatePushConstantIfNeed(AbsPipelineWrapper* pipeline) {
  if (!global_push_const_.dirty) {
    return;
  }

  global_push_const_.dirty = false;

  pipeline->UploadPushConstant(global_push_const_.value, GetCurrentCMD());
}

void VkRenderer::UpdateTransformMatrixIfNeed(AbsPipelineWrapper* pipeline) {
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

void VkRenderer::UpdateCommonSetIfNeed(AbsPipelineWrapper* pipeline) {
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

void VkRenderer::UpdateStencilConfigIfNeed(AbsPipelineWrapper* pipeline) {
  pipeline->UpdateStencilInfo(stencil_value_, stencil_compare_mask_,
                              stencil_write_mask_, ctx_);
}

void VkRenderer::UpdateColorInfoIfNeed(AbsPipelineWrapper* pipeline) {
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

  pipeline->UploadBlurInfo({color_mode_, 0, 0, 0}, ctx_, CurrentFrameBuffer(),
                           vk_memory_allocator_.get());

  if (image_texture_) {
    pipeline->UploadImageTexture(image_texture_, ctx_, CurrentFrameBuffer(),
                                 vk_memory_allocator_.get());
    image_texture_ = nullptr;
  }
  // TODO implement font info update
}

void VkRenderer::UpdateFontInfoIfNeed(AbsPipelineWrapper* pipeline) {
  if (font_texture_) {
    if (pipeline->IsComputePipeline()) {
      auto com_pipeline = (ComputePipeline*)pipeline;
      com_pipeline->UploadOutputTexture(font_texture_);
      return;
    }

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

void VkRenderer::ResetUniformDirty() {
  global_push_const_.dirty = true;
  model_matrix_.dirty = true;
  common_fragment_set_.dirty = true;
  color_info_set_.dirty = true;
  gradient_info_set_.dirty = true;
}

SKVkFrameBufferData* VkRenderer::CurrentFrameBuffer() {
  return frame_buffer_[ctx_->GetCurrentBufferIndex()].get();
}

}  // namespace skity