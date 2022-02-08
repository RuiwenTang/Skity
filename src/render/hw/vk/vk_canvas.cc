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

std::unique_ptr<HWRenderer> VKCanvas::CreateRenderer() {
  auto renderer = std::make_unique<VkRenderer>(ctx_);
  renderer->Init();

  vk_renderer_ = renderer.get();

  return renderer;
}

std::unique_ptr<HWTexture> VKCanvas::GenerateTexture() {
  return std::make_unique<VKTexture>(vk_renderer_->GetInterface(),
                                     vk_renderer_->Allocator(), vk_renderer_,
                                     ctx_);
}

std::unique_ptr<HWFontTexture> VKCanvas::GenerateFontTexture(
    Typeface* typeface) {
  return std::make_unique<VKFontTexture>(vk_renderer_->GetInterface(), typeface,
                                         vk_renderer_->Allocator(),
                                         vk_renderer_, ctx_);
}

std::unique_ptr<HWRenderTarget> VKCanvas::GenerateBackendRenderTarget(
    uint32_t width, uint32_t height) {
  auto vk_rt = std::make_unique<VKRenderTarget>(
      vk_renderer_->GetInterface(), width, height, vk_renderer_->Allocator(),
      vk_renderer_, ctx_);

  vk_rt->Init();

  return vk_rt;
}

}  // namespace skity