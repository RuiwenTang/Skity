#ifndef SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP

#include "src/render/hw/hw_canvas.hpp"
#include "src/render/hw/vk/vk_pipeline.hpp"

namespace skity {

class VKCanvas : public HWCanvas {
 public:
  VKCanvas(Matrix mvp, uint32_t width, uint32_t height, float density);
  ~VKCanvas() override = default;

 protected:
  void OnInit(GPUContext* ctx) override;

  std::unique_ptr<HWPipeline> CreatePipeline() override;

  std::unique_ptr<HWTexture> GenerateTexture() override;

  std::unique_ptr<HWFontTexture> GenerateFontTexture(
      Typeface* typeface) override;

 private:
  VKPipeline* vk_pipeline_ = {};
  GPUVkContext* ctx_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP