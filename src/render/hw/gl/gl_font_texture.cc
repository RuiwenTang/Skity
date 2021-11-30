#include "src/render/hw/gl/gl_font_texture.hpp"

namespace skity {

GLFontTexture::GLFontTexture(Typeface* typeface)
    : HWFontTexture(typeface), GLTexture() {}

void GLFontTexture::Init() {
  GLTexture::Init(HWTexture::Type::kColorTexture, HWTexture::Format::kR);

  this->Bind();
  this->GLTexture::Resize(Width(), Height());
  // can we remove this function call ?
  this->UnBind();
}

HWTexture* GLFontTexture::GetHWTexture() { return this; }

void GLFontTexture::OnUploadRegion(uint32_t x, uint32_t y, uint32_t width,
                                   uint32_t height, uint8_t* data) {
  this->Bind();
  this->UploadData(x, y, width, height, data);
}

void GLFontTexture::OnResize(uint32_t new_width, uint32_t new_height) {
  this->Bind();
  this->GLTexture::Resize(new_height, new_height);
}

}  // namespace skity