#include "src/render/hw/gl/gl_render_target.hpp"

#include "src/logging.hpp"
#include "src/render/hw/gl/gl_interface.hpp"
#include "src/render/hw/gl/gl_texture.hpp"

namespace skity {

void GLRenderTarget::OnInit() {
  GL_CALL(GenFramebuffers, 1, &fbo_);

  if (fbo_ == 0) {
    LOG_ERROR("Failed to create gl frame buffer object!!");
    return;
  }

  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, fbo_);

  auto color_buffer = (GLTexture*)HColorBuffer();
  auto stencil_buffer = (GLTexture*)StencilBuffer();

  // bind color attachment
  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_2D, color_buffer->GetTextureID(), 0);

  // bind stencil attachment
  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
          GL_TEXTURE_2D, stencil_buffer->GetTextureID(), 0);

  auto error = GL_CALL(GetError);
  if (error) {
    LOG_ERROR("get gl error : {:X}", error);
  }

  auto state = GL_CALL(CheckFramebufferStatus, GL_FRAMEBUFFER);

  if (state != GL_FRAMEBUFFER_COMPLETE) {
    LOG_ERROR("GL Framebuffer state is {:X}", state);
  }

  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, 0);
}

void GLRenderTarget::OnDestroy() { GL_CALL(DeleteFramebuffers, 1, &fbo_); }

}  // namespace skity