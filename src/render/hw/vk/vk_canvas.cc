#include "src/render/hw/vk/vk_canvas.hpp"

#include <cassert>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

VKCanvas::VKCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : HWCanvas(mvp, width, height, density) {}

void VKCanvas::OnInit(GPUContext* ctx) {
  if (ctx->type != GPUBackendType::kVulkan) {
    LOG_ERROR("GPUContext is not vulkan backend");
    assert(false);
    return;
  }

  ctx_ = (GPUVkContext*)ctx;
  vk_pipeline_ = std::make_unique<VKPipeline>(ctx_);
  vk_pipeline_->Init();
}

HWPipeline* VKCanvas::GetPipeline() { return vk_pipeline_.get(); }

std::unique_ptr<HWTexture> VKCanvas::GenerateTexture() {
  return std::make_unique<VKTexture>(vk_pipeline_->Allocator(),
                                     vk_pipeline_.get(), ctx_);
}

std::unique_ptr<HWFontTexture> VKCanvas::GenerateFontTexture(
    Typeface* typeface) {
  return std::unique_ptr<HWFontTexture>();
}

}  // namespace skity