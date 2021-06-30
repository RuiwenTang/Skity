#include "src/render/gl/draw/gl_fill_color_op.hpp"

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

void GLFillColorOp::OnBeforeDraw() {
  GLDrawMeshOp::OnBeforeDraw();
  shader_->SetColor(r_, g_, b_, a_);
}

}  // namespace skity