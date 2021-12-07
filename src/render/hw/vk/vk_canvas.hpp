#ifndef SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP

#include "src/render/hw/hw_canvas.hpp"

namespace skity {

class VKCanvas : public HWCanvas {
 public:
  VKCanvas(Matrix mvp, uint32_t width, uint32_t height, float density);
  ~VKCanvas() override = default;

 protected:
  void OnInit(GPUContext* ctx) override;

  HWPipeline* GetPipeline() override;

  std::unique_ptr<HWTexture> GenerateTexture() override;

  std::unique_ptr<HWFontTexture> GenerateFontTexture(
      Typeface* typeface) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP