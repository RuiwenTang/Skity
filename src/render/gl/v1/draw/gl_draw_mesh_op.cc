#include "gl_draw_mesh_op.hpp"

#include "src/render/gl/gl_interface.hpp"
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
//  mesh_->UnBindMesh();
}

void GLDrawMeshOp::OnDraw(bool has_clip) {
  if (front_count() > 0) {
    OnBeforeDrawFront();
    DrawFront();
  }
  if (back_count() > 0) {
    OnBeforeDrawBack();
    DrawBack();
  }
}

void GLDrawMeshOp::OnInit() {}

void GLDrawMeshOp::OnBeforeDrawFront() {}

void GLDrawMeshOp::OnBeforeDrawBack() {}

void GLDrawMeshOp::DrawFront() {
  mesh_->BindFrontIndex();

  GLMeshDraw{draw_mode_, front_start(), front_count()}();
}

void GLDrawMeshOp::DrawBack() {
  mesh_->BindBackIndex();

  GLMeshDraw{draw_mode_, back_start(), back_count()}();
}

}  // namespace skity
