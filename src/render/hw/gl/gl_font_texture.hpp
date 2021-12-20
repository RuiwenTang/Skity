#ifndef SKITY_SRC_RENDER_HW_GL_GL_FONT_TEXTURE_HPP
#define SKITY_SRC_RENDER_HW_GL_GL_FONT_TEXTURE_HPP

#include "src/render/hw/gl/gl_texture.hpp"
#include "src/render/hw/hw_font_texture.hpp"

namespace skity {

class GLFontTexture : public HWFontTexture, public GLTexture {
 public:
  GLFontTexture(Typeface* typeface);
  ~GLFontTexture() override = default;

  void Init() override;

  void Destroy() override;

  HWTexture* GetHWTexture() override;

 protected:
  void OnUploadRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                      uint8_t* data) override;

  void OnResize(uint32_t new_width, uint32_t new_height) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_GL_FONT_TEXTURE_HPP