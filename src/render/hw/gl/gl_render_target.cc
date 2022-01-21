#include "src/render/hw/gl/gl_render_target.hpp"

#include "src/logging.hpp"
#include "src/render/hw/gl/gl_interface.hpp"
#include "src/render/hw/gl/gl_texture.hpp"

namespace skity {

void check_fbo_state(GLuint fbo) {
  auto state = GL_CALL(CheckFramebufferStatus, GL_FRAMEBUFFER);

  if (state != GL_FRAMEBUFFER_COMPLETE) {
    LOG_ERROR("GL Framebuffer state is {:X}", state);
  }
}

void GLRenderTarget::OnInit() {
  GL_CALL(GenFramebuffers, 1, &fbo_);

  if (fbo_ == 0) {
    LOG_ERROR("Failed to create gl frame buffer object!!");
    return;
  }

  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, fbo_);

  auto stencil_buffer = (GLTexture*)StencilBuffer();

  // bind stencil attachment
  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
          GL_TEXTURE_2D, stencil_buffer->GetTextureID(), 0);

  auto error = GL_CALL(GetError);
  if (error) {
    LOG_ERROR("get gl error : {:X}", error);
  }

  check_fbo_state(fbo_);

  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, 0);
}

void GLRenderTarget::Bind() { GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, fbo_); }

void GLRenderTarget::BindHBuffer() {
  auto error = GL_CALL(GetError);

  Bind();

  auto texture = (GLTexture*)HColorBuffer();

  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_2D, texture->GetTextureID(), 0);

  check_fbo_state(fbo_);
  Clear();
}

void GLRenderTarget::BindVBuffer() {
  Bind();

  auto texture = (GLTexture*)VColorBuffer();

  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_2D, texture->GetTextureID(), 0);

  check_fbo_state(fbo_);

  Clear();
}

void GLRenderTarget::UnBind() {
  check_fbo_state(fbo_);
  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, 0);
}

void GLRenderTarget::OnDestroy() { GL_CALL(DeleteFramebuffers, 1, &fbo_); }

void GLRenderTarget::Clear() {
  static const GLenum color_buffer = GL_COLOR_ATTACHMENT0;
  static const GLenum stencil_buffer = GL_DEPTH_STENCIL_ATTACHMENT;

  static const float transparent[] = {0.f, 0.f, 0.f, 0.f};
  static const int32_t zero = 0;

  // clear color
  GL_CALL(DrawBuffers, 1, &color_buffer);
  GL_CALL(ClearBufferfv, GL_COLOR, 0, transparent);

  // clear stencil
  GL_CALL(ClearBufferfi, GL_STENCIL, 0, 0.f, 0);
}

}  // namespace skity