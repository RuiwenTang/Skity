#include "src/render/gl/draw/gl_stencil_op.hpp"

#include <glad/glad.h>

#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLStencilDrawOp::GLStencilDrawOp(uint32_t front_start, uint32_t front_count,
                                 uint32_t back_start, uint32_t back_count,
                                 StencilShader* shader, GLMesh* mesh,
                                 bool positive)
    : GLDrawMeshOp(front_start, front_count, back_start, back_count, shader,
                   mesh),
      shader_(shader),
      postive_(positive) {
  UpdateStencilValues();
}

void GLStencilDrawOp::UpdateStrokeWidth(float width) {
  shader_->SetStrokeRadius(width / 2.f);
}

void GLStencilDrawOp::UpdateStencilValues() {
  if (postive_) {
    SetStencilFrontFlag(GL_INCR_WRAP);
    SetStencilBackFlag(GL_DECR_WRAP);
  } else {
    SetStencilFrontFlag(GL_DECR_WRAP);
    SetStencilBackFlag(GL_INCR_WRAP);
  }
}

}  // namespace skity