#include "src/render/gl/draw/gl_fill_color_op.hpp"

#include <glad/glad.h>

#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLFillColorOp::GLFillColorOp(uint32_t front_start, uint32_t front_count,
                             uint32_t back_start, uint32_t back_count,
                             ColorShader* shader, GLMesh* mesh)
    : GLDrawMeshOp(front_start, front_count, back_start, back_count, shader,
                   mesh),
      shader_(shader) {}

void GLFillColorOp::SetColor(float r, float g, float b, float a) {
  r_ = r;
  g_ = g;
  b_ = b;
  a_ = a;
}

void GLFillColorOp::OnBeforeDraw(bool has_clip) {
  GLDrawMeshOp::OnBeforeDraw(has_clip);
  shader_->SetColor(r_, g_, b_, a_);

  glEnable(GL_STENCIL_TEST);
  glColorMask(1, 1, 1, 1);
  if (has_clip) {
    glStencilMask(0x1F);
    glStencilFunc(GL_LESS, 0x10, 0x1F);
  } else {
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
  }
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void GLFillColorOp::OnAfterDraw(bool has_clip) {
  GLDrawMeshOp::OnAfterDraw(has_clip);

  glDisable(GL_STENCIL_TEST);
  glColorMask(0, 0, 0, 0);
}

}  // namespace skity