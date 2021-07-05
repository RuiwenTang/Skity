#include "src/render/gl/draw/gl_draw_mesh_op.hpp"

#include <glad/glad.h>

#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLDrawMeshOp::GLDrawMeshOp(uint32_t front_start, uint32_t front_count,
                           uint32_t back_start, uint32_t back_count,
                           GLShader* shader, GLMesh* mesh)
    : GLDrawOp(front_start, front_count, back_start, back_count, shader),
      mesh_(mesh) {}

void GLDrawMeshOp::OnBeforeDraw() {
  GLDrawOp::OnBeforeDraw();
  mesh_->BindMesh();
}

void GLDrawMeshOp::OnAfterDraw() {
  GLDrawOp::OnAfterDraw();
  mesh_->UnBindMesh();
}

void GLDrawMeshOp::OnDraw() {
  OnBeforeDrawFront();
  DrawFront();
  OnBeforeDrawBack();
  DrawBack();
}

void GLDrawMeshOp::OnInit() {}

void GLDrawMeshOp::OnBeforeDrawFront() {}

void GLDrawMeshOp::OnBeforeDrawBack() {}

void GLDrawMeshOp::DrawFront() {
  mesh_->BindFrontIndex();

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void*)(2 * sizeof(float)));

  glDrawElements(GL_TRIANGLES, front_count(), GL_UNSIGNED_INT,
                 (void*)(front_start() * sizeof(GLuint)));
}

void GLDrawMeshOp::DrawBack() {
  mesh_->BindBackIndex();

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void*)(2 * sizeof(float)));

  glDrawElements(GL_TRIANGLES, back_count(), GL_UNSIGNED_INT,
                 (void*)(back_start() * sizeof(GLuint)));
}

}  // namespace skity