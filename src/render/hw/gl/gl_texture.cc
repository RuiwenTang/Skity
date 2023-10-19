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
    case HWTexture::Format::kS:
      return GL_DEPTH_STENCIL;
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
  if (format_ != GL_DEPTH_STENCIL && msaa_count_ == 0) {
    // texture common config
    GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (format == HWTexture::Format::kR) {
      GL_CALL(PixelStorei, GL_UNPACK_ALIGNMENT, 1);
    }
  }

  UnBind();
}

void GLTexture::Destroy() { GL_CALL(DeleteTextures, 1, &texture_id_); }

void GLTexture::Bind() {
  if (msaa_count_ > 0) {
    GL_CALL(BindTexture, GL_TEXTURE_2D_MULTISAMPLE, texture_id_);
  } else {
    GL_CALL(BindTexture, GL_TEXTURE_2D, texture_id_);
  }
}

void GLTexture::UnBind() {
  if (msaa_count_) {
    GL_CALL(BindTexture, GL_TEXTURE_2D_MULTISAMPLE, 0);
  } else {
    GL_CALL(BindTexture, GL_TEXTURE_2D, 0);
  }
}

uint32_t GLTexture::GetWidth() { return width_; }

uint32_t GLTexture::GetHeight() { return height_; }

void GLTexture::Resize(uint32_t width, uint32_t height) {
  width_ = width;
  height_ = height;
  if (msaa_count_ > 0) {
    GL_CALL(TexImage2DMultisample, GL_TEXTURE_2D_MULTISAMPLE, msaa_count_,
            GetInternalFormat(), width, height, GL_TRUE);
  } else {
    GL_CALL(TexImage2D, GL_TEXTURE_2D, 0, GetInternalFormat(), width, height, 0,
            format_, GetInternalType(), nullptr);
  }
}

void GLTexture::UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                           uint32_t height, size_t row_bytes,void *data) {
  if (msaa_count_ > 0) {
    return;
  }

  GL_CALL(TexSubImage2D, GL_TEXTURE_2D, 0, offset_x, offset_y, width, height,
          format_, GetInternalType(), data);
}

uint32_t GLTexture::GetInternalType() const {
  if (format_ == GL_DEPTH_STENCIL) {
    return GL_UNSIGNED_INT_24_8;
  } else {
    return GL_UNSIGNED_BYTE;
  }
}

int32_t GLTexture::GetInternalFormat() const {
  if (format_ == GL_RED) {
    return GL_R8;
  } else if (format_ == GL_RGB) {
    return GL_RGB;
  } else if (format_ == GL_RGBA) {
    return GL_RGBA8;
  } else if (format_ == GL_DEPTH_STENCIL) {
    return GL_DEPTH24_STENCIL8;
  }

  return GL_RGBA8;
}

}  // namespace skity
