#include "src/render/hw/vk/vk_pipeline.hpp"

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"

#define SKITY_DEFAULT_BUFFER_SIZE 512

namespace skity {

VKPipeline::VKPipeline(GPUVkContext* ctx)
    : HWPipeline(),
      ctx_(ctx),
      vk_memory_allocator_(VKMemoryAllocator::CreateMemoryAllocator()) {}

VKPipeline::~VKPipeline() {
  vk_memory_allocator_->FreeBuffer(vertex_buffer_.get());
  vk_memory_allocator_->FreeBuffer(index_buffer_.get());
  static_color_pipeline_->Destroy(ctx_);

  DestroyFrameBuffers();
  vk_memory_allocator_->Destroy(ctx_);
}

void VKPipeline::Init() {
  if (!VKInterface::GlobalInterface()) {
    VKInterface::InitGlobalInterface(
        ctx_->GetDevice(), (PFN_vkGetDeviceProcAddr)ctx_->proc_loader);
  }
  vk_memory_allocator_->Init(ctx_);
  InitFrameBuffers();
  InitPipelines();
}

void VKPipeline::Bind() {
  LOG_DEBUG("vk_pipeline Bind");
  prev_pipeline_ = nullptr;

  CurrentFrameBuffer()->FrameBegin(ctx_);
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
}

void VKPipeline::SetUniformColor(const glm::vec4& color) {
  LOG_DEBUG("vk_pipeline set uniform color");
  color_info_set_.value.user_color = color;
  color_info_set_.dirty = true;
}

void VKPipeline::SetGradientBoundInfo(const glm::vec4& info) {
  LOG_DEBUG("vk_pipeline set gradient bounds");
}

void VKPipeline::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {
  LOG_DEBUG("vk_pipeline set gradient color and stop count");
}

void VKPipeline::SetGradientColors(const std::vector<Color4f>& colors) {
  LOG_DEBUG("vk_pipeline set gradient colors");
}

void VKPipeline::SetGradientPositions(const std::vector<float>& pos) {
  LOG_DEBUG("vk_pipeline set gradient stops");
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
  common_fragment_set_.value.global_alpha = alpha;
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
    // TODO implement pick stencil pipeline
  } else if (color_mode_ == HWPipelineColorMode::kUniformColor) {
    picked_pipeline = PickColorPipeline();
  } else if (color_mode_ == HWPipelineColorMode::kImageTexture) {
    // TODO implement pick image pipeline
  } else if (color_mode_ == HWPipelineColorMode::kLinearGradient ||
             color_mode_ == HWPipelineColorMode::kRadialGradient) {
    // TODO implement pick gradient pipeline
  }

  BindPipelineIfNeed(picked_pipeline);

  UpdatePushConstantIfNeed(picked_pipeline);
  UpdateTransformMatrixIfNeed(picked_pipeline);
  UpdateStencilConfigIfNeed(picked_pipeline);
  UpdateColorInfoIfNeed(picked_pipeline);
}

void VKPipeline::BindTexture(HWTexture* texture, uint32_t slot) {
  LOG_DEBUG("vk_pipeline bind to {}", slot);
}

void VKPipeline::InitFrameBuffers() {
  frame_buffer_.resize(ctx_->GetSwapchainBufferCount());
  for (size_t i = 0; i < frame_buffer_.size(); i++) {
    frame_buffer_[i] =
        std::make_unique<VKFrameBuffer>(vk_memory_allocator_.get());
    frame_buffer_[i]->Init(ctx_);
  }
}

void VKPipeline::DestroyFrameBuffers() {
  for (size_t i = 0; i < frame_buffer_.size(); i++) {
    frame_buffer_[i]->Destroy(ctx_);
  }
}

void VKPipeline::InitPipelines() {
  static_color_pipeline_ = VKPipelineWrapper::CreateStaticColorPipeline(ctx_);
}

void VKPipeline::InitVertexBuffer(size_t new_size) {
  vertex_buffer_.reset(vk_memory_allocator_->AllocateVertexBuffer(new_size));
}

void VKPipeline::InitIndexBuffer(size_t new_size) {
  index_buffer_.reset(vk_memory_allocator_->AllocateIndexBuffer(new_size));
}

VKPipelineWrapper* VKPipeline::PickColorPipeline() {
  if (!enable_stencil_test_) {
    return static_color_pipeline_.get();
  }

  return nullptr;
}

void VKPipeline::BindPipelineIfNeed(VKPipelineWrapper* pipeline) {
  // TODO implement pipeline bind
}

void VKPipeline::UpdatePushConstantIfNeed(VKPipelineWrapper* pipeline) {
  // TODO implement push constant update
}

void VKPipeline::UpdateTransformMatrixIfNeed(VKPipelineWrapper* pipeline) {
  // TODO implement transform matrix update
}

void VKPipeline::UpdateStencilConfigIfNeed(VKPipelineWrapper* pipeline) {
  // TODO implement stencil info update
}

void VKPipeline::UpdateColorInfoIfNeed(VKPipelineWrapper* pipeline) {
  // TODO implement color info update
}

VKFrameBuffer* VKPipeline::CurrentFrameBuffer() {
  return frame_buffer_[ctx_->GetCurrentBufferIndex()].get();
}

}  // namespace skity