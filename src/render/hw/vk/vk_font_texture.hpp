#ifndef SKITY_SRC_RENDER_HW_VK_VK_FONT_TEXTURE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_FONT_TEXTURE_HPP

#include "src/render/hw/hw_font_texture.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

class VKFontTexture : public HWFontTexture, public VKTexture {
 public:
  VKFontTexture(Typeface* typeface, VKMemoryAllocator* allocator,
                SKVkPipelineImpl* pipeline, GPUVkContext* ctx);
  ~VKFontTexture() override;

  void Init() override;

  void Destroy() override;

  HWTexture* GetHWTexture() override;

 protected:
  void OnUploadRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                      uint8_t* data) override;

  void OnResize(uint32_t new_width, uint32_t new_height) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_FONT_TEXTURE_HPP