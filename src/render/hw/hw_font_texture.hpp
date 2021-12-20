#ifndef SKITY_SRC_RENDER_HW_HW_FONT_TEXTURE_HPP
#define SKITY_SRC_RENDER_HW_HW_FONT_TEXTURE_HPP

#include "src/render/text/font_texture.hpp"

namespace skity {

class HWTexture;

class HWFontTexture : public FontTexture {
 public:
  HWFontTexture(Typeface* typeface) : FontTexture(typeface) {}
  ~HWFontTexture() override = default;

  virtual void Init() = 0;
  virtual void Destroy() = 0;

  virtual HWTexture* GetHWTexture() = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_FONT_TEXTURE_HPP