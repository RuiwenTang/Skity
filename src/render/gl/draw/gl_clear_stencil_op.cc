#include "src/render/gl/draw/gl_clear_stencil_op.hpp"

#include <glad/glad.h>

namespace skity {

GLClearStencilOp::GLClearStencilOp() : GLDrawOp(0, 0, 0, 0, nullptr) {}

void GLClearStencilOp::OnBeforeDraw(bool has_clip) {}

void GLClearStencilOp::OnAfterDraw(bool has_clip) {}

void GLClearStencilOp::OnDraw(bool has_clip) { glClear(GL_STENCIL_BUFFER_BIT); }

void GLClearStencilOp::OnInit() {}

}  // namespace skity