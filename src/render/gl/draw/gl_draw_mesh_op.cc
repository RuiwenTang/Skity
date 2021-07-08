#include "src/render/gl/draw/gl_draw_mesh_op.hpp"

#include <glad/glad.h>

#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLDrawMeshOp::GLDrawMeshOp(uint32_t front_start, uint32_t front_count,
                           uint32_t back_start, uint32_t back_count,
                           GLShader* shader, GLMesh* mesh)
    : GLDrawOp(front_start, front_count, back_start, back_count, shader),
      mesh_(mesh),
      draw_mode_(GL_TRIANGLES) {}

void GLDrawMeshOp::OnBeforeDraw(bool has_clip) {
  GLDrawOp::OnBeforeDraw(has_clip);
  mesh_->BindMesh();
}

void GLDrawMeshOp::OnAfterDraw(bool has_clip) {
  GLDrawOp::OnAfterDraw(has_clip);
  mesh_->UnBindMesh();
}

void GLDrawMeshOp::OnDraw(bool has_clip) {
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

  glDrawElements(draw_mode_, front_count(), GL_UNSIGNED_INT,
                 (void*)(front_start() * sizeof(GLuint)));
}

void GLDrawMeshOp::DrawBack() {
  mesh_->BindBackIndex();

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void*)(2 * sizeof(float)));

  glDrawElements(draw_mode_, back_count(), GL_UNSIGNED_INT,
                 (void*)(back_start() * sizeof(GLuint)));
}

}  // namespace skity