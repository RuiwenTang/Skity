#include "src/render/hw/vk/vk_canvas.hpp"

namespace skity {

VKCanvas::VKCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : HWCanvas(mvp, width, height, density) {}

void VKCanvas::OnInit(GPUContext* ctx) {}

HWPipeline* VKCanvas::GetPipeline() { return nullptr; }

std::unique_ptr<HWTexture> VKCanvas::GenerateTexture() {
  return std::unique_ptr<HWTexture>();
}

std::unique_ptr<HWFontTexture> VKCanvas::GenerateFontTexture(
    Typeface* typeface) {
  return std::unique_ptr<HWFontTexture>();
}

}  // namespace skity