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

  vk_memory_allocator_->Destroy(ctx_);
}

void VKPipeline::Init() {
  if (!VKInterface::GlobalInterface()) {
    VKInterface::InitGlobalInterface(
        ctx_->GetDevice(), (PFN_vkGetDeviceProcAddr)ctx_->proc_loader);
  }
  vk_memory_allocator_->Init(ctx_);
  InitPipelines();
}

void VKPipeline::Bind() { LOG_DEBUG("vk_pipeline Bind"); }

void VKPipeline::UnBind() { LOG_DEBUG("vk_pipeline UnBind"); }

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
    InitIndexBuffer(new_size);
  }

  vk_memory_allocator_->UploadBuffer(index_buffer_.get(), data, data_size);

  VK_CALL(vkCmdBindIndexBuffer, ctx_->GetCurrentCMD(),
          index_buffer_->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void VKPipeline::SetGlobalAlpha(float alpha) {
  LOG_DEBUG("vk_pipeline set global alpha");
  color_info_set_.value.global_alpha = alpha;
  color_info_set_.dirty = true;
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
}

void VKPipeline::BindTexture(HWTexture* texture, uint32_t slot) {
  LOG_DEBUG("vk_pipeline bind to {}", slot);
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

}  // namespace skity