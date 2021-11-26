#ifndef SKITY_SRC_RENDER_HW_GL_GL_TEXTURE_HPP
#define SKITY_SRC_RENDER_HW_GL_GL_TEXTURE_HPP

#include "src/render/hw/hw_texture.hpp"

namespace skity {

class GLTexture : public HWTexture {
 public:
  GLTexture() = default;
  ~GLTexture() override;

  void Init(HWTexture::Type type, HWTexture::Format format) override;

  void Bind() override;
  void UnBind() override;

  uint32_t GetWidth() override;
  uint32_t GetHeight() override;

  void Resize(uint32_t width, uint32_t height) override;

  void UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                  uint32_t height, void* data) override;

 private:
  uint32_t texture_id_ = 0;
  uint32_t format_ = 0;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_GL_TEXTURE_HPP