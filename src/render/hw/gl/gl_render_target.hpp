#ifndef SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP

#include "src/render/hw/gl/gl_texture.hpp"
#include "src/render/hw/hw_render_target.hpp"

namespace skity {

class GLRenderTarget : public HWRenderTarget {
 public:
  GLRenderTarget(uint32_t width, uint32_t height)
      : HWRenderTarget(width, height), fbo_(0) {}
  ~GLRenderTarget() override = default;

  uint32_t GetFrameBufferID() const { return fbo_; }

  HWTexture* ColorTexture() override { return &color_texture_; }

  HWTexture* HorizontalTexture() override { return &horizontal_texture_; }

  HWTexture* VerticalTexture() override { return &vertical_texture_; }

  void BindColorTexture() override;

  void BlitColorTexture() override;

  void BindHorizontalTexture() override;

  void BindVerticalTexture() override;

  void Init() override;

  void Destroy() override;

  void Bind();

  void UnBind();

 private:
  void InitTextures();
  void InitFBO();
  void InitMultiSampleTexture();

  void Clear();

 private:
  uint32_t fbo_ = {};
  uint32_t msaa_fbo_ = {};
  uint32_t msaa_sample_count_ = 8;
  uint32_t msaa_target_ = {};
  GLTexture color_texture_;
  GLTexture horizontal_texture_;
  GLTexture vertical_texture_;
  GLTexture stencil_texture_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP