#include "src/render/hw/vk/vk_canvas.hpp"

#include <cassert>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_font_texture.hpp"
#include "src/render/hw/vk/vk_render_target.hpp"
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
}

std::unique_ptr<HWPipeline> VKCanvas::CreatePipeline() {
  auto pipeline = std::make_unique<SKVkPipelineImpl>(ctx_);
  pipeline->Init();

  vk_pipeline_ = pipeline.get();

  return pipeline;
}

std::unique_ptr<HWTexture> VKCanvas::GenerateTexture() {
  return std::make_unique<VKTexture>(vk_pipeline_->Allocator(), vk_pipeline_,
                                     ctx_);
}

std::unique_ptr<HWFontTexture> VKCanvas::GenerateFontTexture(
    Typeface* typeface) {
  return std::make_unique<VKFontTexture>(typeface, vk_pipeline_->Allocator(),
                                         vk_pipeline_, ctx_);
}

std::unique_ptr<HWRenderTarget> VKCanvas::GenerateBackendRenderTarget(
    uint32_t width, uint32_t height) {
  auto vk_rt = std::make_unique<VKRenderTarget>(
      width, height, vk_pipeline_->Allocator(), vk_pipeline_, ctx_);

  vk_rt->Init();

  return vk_rt;
}

}  // namespace skity