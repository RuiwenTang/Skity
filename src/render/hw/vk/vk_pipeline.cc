#include "src/render/hw/vk/vk_pipeline.hpp"

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"

namespace skity {

VKPipeline::VKPipeline(GPUVkContext* ctx)
    : HWPipeline(),
      ctx_(ctx),
      vk_memory_allocator_(VKMemoryAllocator::CreateMemoryAllocator()) {}

VKPipeline::~VKPipeline() { static_color_pipeline_->Destroy(ctx_); }

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
}

void VKPipeline::SetModelMatrix(const glm::mat4& matrix) {
  LOG_DEBUG("vk_pipeline upload transform matrix");
}

void VKPipeline::SetPipelineColorMode(HWPipelineColorMode mode) {
  LOG_DEBUG("vk_pipeline set color mode");
}

void VKPipeline::SetStrokeWidth(float width) {
  LOG_DEBUG("vk_pipeline set stroke width");
}

void VKPipeline::SetUniformColor(const glm::vec4& color) {
  LOG_DEBUG("vk_pipeline set uniform color");
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
}

void VKPipeline::UploadIndexBuffer(void* data, size_t data_size) {
  LOG_DEBUG("vk_pipeline upload index buffer with size: {}", data_size);
}

void VKPipeline::SetGlobalAlpha(float alpha) {
  LOG_DEBUG("vk_pipeline set global alpha");
}

void VKPipeline::EnableStencilTest() {
  LOG_DEBUG("vk_pipeline enable stencil test");
}

void VKPipeline::DisableStencilTest() {
  LOG_DEBUG("vk_pipeline disable stencil test");
}

void VKPipeline::EnableColorOutput() {
  LOG_DEBUG("vk_pipeline enable color output");
}

void VKPipeline::DisableColorOutput() {
  LOG_DEBUG("vk_pipeline disable color output");
}

void VKPipeline::UpdateStencilMask(uint8_t write_mask) {
  LOG_DEBUG("vk_pipeline set stencil write mask {:x}", write_mask);
}

void VKPipeline::UpdateStencilOp(HWStencilOp op) {
  LOG_DEBUG("vk_pipeline set stencil op");
}

void VKPipeline::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                   uint32_t compare_mask) {
  LOG_DEBUG("vk_pipeline set stencil func with value : {} ; mask : {:x}", value,
            compare_mask);
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

}  // namespace skity