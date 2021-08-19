#include "src/render/gl/gl_texture.hpp"

#include "src/render/gl/gl_interface.hpp"

namespace skity {

GLTexture::~GLTexture() {
  if (texture_id_ > 0) {
    GL_CALL(DeleteTextures, 1, (const GLuint*)&texture_id_);
  }
}

void GLTexture::Bind() const {
  GL_CALL(BindTexture, GL_TEXTURE_2D, texture_id_);
}

void GLTexture::UnBind() const { GL_CALL(BindTexture, GL_TEXTURE_2D, 0); }

bool GLTexture::InitTexture() {
  if (pixmap_ == nullptr) {
    texture_id_ = 0;
    return false;
  }

  GL_CALL(GenTextures, 1, (GLuint*)&texture_id_);

  // setup texture arguments
  Bind();

  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  GL_CALL(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GL_CALL(TexImage2D, GL_TEXTURE_2D, 0, GL_RGBA, pixmap_->Width(),
          pixmap_->Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixmap_->Addr());

  UnBind();

  return true;
}

const GLTexture* GLTextureManager::GenerateTexture(const Pixmap* pixmap) {
  auto it = texture_store_.find(pixmap);
  if (it != texture_store_.end()) {
    return it->second.get();
  }

  std::unique_ptr<GLTexture> texture(new GLTexture(pixmap));

  if (!texture->InitTexture()) {
    return nullptr;
  }

  texture_store_[pixmap] = std::move(texture);

  return texture_store_[pixmap].get();
}

}  // namespace skity