#ifndef SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP

#include "src/render/hw/hw_canvas.hpp"
#include "src/render/hw/vk/vk_renderer.hpp"

namespace skity {

class VKCanvas : public HWCanvas {
 public:
  VKCanvas(Matrix mvp, uint32_t width, uint32_t height, float density);
  ~VKCanvas() override = default;

 protected:
  void OnInit(GPUContext* ctx) override;

  bool SupportGeometryShader() override;

  std::unique_ptr<HWRenderer> CreateRenderer() override;

  std::unique_ptr<HWTexture> GenerateTexture() override;

  std::unique_ptr<HWFontTexture> GenerateFontTexture(
      Typeface* typeface) override;

  std::unique_ptr<HWRenderTarget> GenerateBackendRenderTarget(
      uint32_t width, uint32_t height) override;

  void onFlush() override;

 private:
  void HandleOrientation();

 private:
  VkRenderer* vk_renderer_ = {};
  GPUVkContext* ctx_ = {};
  VkPhysicalDeviceFeatures vk_phy_features_ = {};
  VkSurfaceTransformFlagBitsKHR current_transform_ =
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_CANVAS_HPP