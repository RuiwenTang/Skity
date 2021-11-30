#include "src/render/hw/gl/gl_texture.hpp"

#include "src/render/hw/gl/gl_interface.hpp"

namespace skity {

static GLenum hw_texture_format_to_gl(HWTexture::Format format) {
  switch (format) {
    case HWTexture::Format::kR:
      return GL_RED;
    case HWTexture::Format::kRGB:
      return GL_RGB;
    case HWTexture::Format::kRGBA:
      return GL_RGBA;
  }

  return GL_RGBA;
}

GLTexture::~GLTexture() {
  if (texture_id_) {
    GL_CALL(DeleteTextures, 1, &texture_id_);
  }
}

void GLTexture::Init(HWTexture::Type type, HWTexture::Format format) {
  // generate texture
  GL_CALL(GenTextures, 1, &texture_id_);

  format_ = hw_texture_format_to_gl(format);

  Bind();

  // texture common config
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  UnBind();
}

void GLTexture::Bind() { GL_CALL(BindTexture, GL_TEXTURE_2D, texture_id_); }

void GLTexture::UnBind() { GL_CALL(BindTexture, GL_TEXTURE_2D, 0); }

uint32_t GLTexture::GetWidth() { return width_; }

uint32_t GLTexture::GetHeight() { return height_; }

void GLTexture::Resize(uint32_t width, uint32_t height) {
  width_ = width;
  height_ = height;

  GL_CALL(TexImage2D, GL_TEXTURE_2D, 0, format_, width, height, 0, format_,
          GL_UNSIGNED_BYTE, nullptr);
}

void GLTexture::UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                           uint32_t height, void *data) {
  GL_CALL(TexSubImage2D, GL_TEXTURE_2D, 0, offset_x, offset_y, width, height,
          format_, GL_UNSIGNED_BYTE, data);
}

}  // namespace skity