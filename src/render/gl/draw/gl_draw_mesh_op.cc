#include "src/render/gl/draw/gl_draw_mesh_op.hpp"

#include <glad/glad.h>

#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLDrawMeshOp::GLDrawMeshOp(uint32_t front_start, uint32_t front_count,
                           uint32_t back_start, uint32_t back_count,
                           GLShader* shader, GLMesh* mesh)
    : GLDrawOp(front_start, front_count, back_start, back_count),

      stencil_value_(0x01),
      stencil_mask_(0x0F),
      front_flag_(GL_INCR_WRAP),
      back_flag_(GL_DECR_WRAP),
      shader_(shader),
      mesh_(mesh) {}

void GLDrawMeshOp::OnBeforeDraw() {}

void GLDrawMeshOp::OnDraw() {
  mesh_->BindMesh();
  shader_->Bind();
  OnBeforeDrawFront();
  DrawFront();
  OnBeforeDrawBack();
  DrawBack();
  shader_->UnBind();
  mesh_->UnBindMesh();
}

void GLDrawMeshOp::OnInit() {}

void GLDrawMeshOp::OnBeforeDrawFront() {}

void GLDrawMeshOp::OnBeforeDrawBack() {}

void GLDrawMeshOp::DrawFront() {
  mesh_->BindFrontIndex();

  glDrawElements(GL_TRIANGLES, front_count(), GL_UNSIGNED_INT,
                 (void*)(front_start() * sizeof(GLuint)));
}

void GLDrawMeshOp::DrawBack() {
  mesh_->BindFrontIndex();

  glDrawElements(GL_TRIANGLES, back_count(), GL_UNSIGNED_INT,
                 (void*)(back_start() * sizeof(GLuint)));
}

}  // namespace skity