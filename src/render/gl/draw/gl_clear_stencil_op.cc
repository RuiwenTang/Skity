#include "src/render/gl/draw/gl_clear_stencil_op.hpp"

#include <glad/glad.h>

namespace skity {

GLClearStencilOp::GLClearStencilOp() : GLDrawOp(0, 0, 0, 0) {}

void GLClearStencilOp::OnBeforeDraw() {}

void GLClearStencilOp::OnAfterDraw() {}

void GLClearStencilOp::OnDraw() { glClear(GL_STENCIL_BUFFER_BIT); }

void GLClearStencilOp::OnInit() {}

}  // namespace skity