#include "src/render/hw/vk/vk_pipeline.hpp"

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"

namespace skity {

VKPipeline::VKPipeline(GPUVkContext* ctx) : HWPipeline(), ctx_(ctx) {}

VKPipeline::~VKPipeline() { static_color_pipeline_->Destroy(ctx_); }

void VKPipeline::Init() {
  if (!VKInterface::GlobalInterface()) {
    VKInterface::InitGlobalInterface(
        ctx_->GetDevice(), (PFN_vkGetDeviceProcAddr)ctx_->proc_loader);
  }
  InitPipelines();
}

void VKPipeline::Bind() {}

void VKPipeline::UnBind() {}

void VKPipeline::SetViewProjectionMatrix(const glm::mat4& mvp) {}

void VKPipeline::SetModelMatrix(const glm::mat4& matrix) {}

void VKPipeline::SetPipelineColorMode(HWPipelineColorMode mode) {}

void VKPipeline::SetStrokeWidth(float width) {}

void VKPipeline::SetUniformColor(const glm::vec4& color) {}

void VKPipeline::SetGradientBoundInfo(const glm::vec4& info) {}

void VKPipeline::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {}

void VKPipeline::SetGradientColors(const std::vector<Color4f>& colors) {}

void VKPipeline::SetGradientPositions(const std::vector<float>& pos) {}

void VKPipeline::UploadVertexBuffer(void* data, size_t data_size) {}

void VKPipeline::UploadIndexBuffer(void* data, size_t data_size) {}

void VKPipeline::SetGlobalAlpha(float alpha) {}

void VKPipeline::EnableStencilTest() {}

void VKPipeline::DisableStencilTest() {}

void VKPipeline::EnableColorOutput() {}

void VKPipeline::DisableColorOutput() {}

void VKPipeline::UpdateStencilMask(uint8_t write_mask) {}

void VKPipeline::UpdateStencilOp(HWStencilOp op) {}

void VKPipeline::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                   uint32_t compare_mask) {}

void VKPipeline::DrawIndex(uint32_t start, uint32_t count) {}

void VKPipeline::BindTexture(HWTexture* texture, uint32_t slot) {}

void VKPipeline::InitPipelines() {
  static_color_pipeline_ = VKPipelineWrapper::CreateStaticColorPipeline(ctx_);
}

}  // namespace skity