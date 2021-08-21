#ifndef SRC_RENDER_GL_GL_TEXTURE_HPP
#define SRC_RENDER_GL_GL_TEXTURE_HPP

#include <map>
#include <memory>
#include <skity/codec/pixmap.hpp>

namespace skity {

class GLTexture final {
 public:
  ~GLTexture();

  bool InitTexture();

  void Bind() const;

  void UnBind() const;

 private:
  GLTexture(const Pixmap* pixmap) : pixmap_(pixmap) {}

 private:
  int32_t texture_id_ = -1;
  const Pixmap* pixmap_ = nullptr;

  friend class GLTextureManager;
};

class GLTextureManager final {
 public:
  GLTextureManager() = default;

  ~GLTextureManager() = default;

  const GLTexture* GenerateTexture(const Pixmap* pixmap);

 private:
  std::map<const Pixmap*, std::unique_ptr<GLTexture>> texture_store_ = {};
};

}  // namespace skity

#endif  // SRC_RENDER_GL_GL_TEXTURE_HPP