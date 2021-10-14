#include "gl_clear_stencil_op.hpp"

#include "src/render/gl/gl_interface.hpp"

namespace skity {

GLClearStencilOp::GLClearStencilOp() : GLDrawOp(0, 0, 0, 0, nullptr) {}

void GLClearStencilOp::OnBeforeDraw(bool has_clip) {}

void GLClearStencilOp::OnAfterDraw(bool has_clip) {}

void GLClearStencilOp::OnDraw(bool has_clip) {
  GL_CALL(Clear, GL_STENCIL_BUFFER_BIT);
}

void GLClearStencilOp::OnInit() {}

}  // namespace skity