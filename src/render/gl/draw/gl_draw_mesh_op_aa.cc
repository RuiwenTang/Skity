#include "src/render/gl/draw/gl_draw_mesh_op_aa.hpp"

#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"

namespace skity {

GLDrawMeshOpAA::GLDrawMeshOpAA(uint32_t front_start, uint32_t front_count,
                               uint32_t back_start, uint32_t back_count,
                               uint32_t aa_start, uint32_t aa_count,
                               GLShader* shader, GLMesh* mesh)
    : GLDrawMeshOp(front_start, front_count, back_start, back_count, shader,
                   mesh),
      mesh_(mesh),
      aa_start_(aa_start),
      aa_count_(aa_count) {}

void GLDrawMeshOpAA::OnDraw(bool has_clip) {
  // Fixme: solve aa_outline cover inner mesh color, maybe this code will
  // rewrite in future
  if (aa_count_ > 0) {
    OnBeforeDrawAAOutline(has_clip);
    DrawAAOutline();
  }
  OnBeforeDraw(has_clip);
  GLDrawMeshOp::OnDraw(has_clip);
}

void GLDrawMeshOpAA::OnBeforeDraw(bool has_clip) {
  GLDrawMeshOp::OnBeforeDraw(has_clip);
  GL_CALL(Enable, GL_STENCIL_TEST);
  GL_CALL(ColorMask, 1, 1, 1, 1);
  if (has_clip) {
    GL_CALL(StencilMask, 0xF);
    GL_CALL(StencilFunc, GL_LESS, 0x10, 0x1F);
  } else {
    GL_CALL(StencilMask, 0x0F);
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x00, 0x0F);
  }
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);
}

void GLDrawMeshOpAA::OnAfterDraw(bool has_clip) {
  GLDrawMeshOp::OnAfterDraw(has_clip);
  GL_CALL(Disable, GL_STENCIL_TEST);
  GL_CALL(ColorMask, 0, 0, 0, 0);
}

void GLDrawMeshOpAA::OnBeforeDrawAAOutline(bool has_clip) {
  GL_CALL(Enable, GL_STENCIL_TEST);
  GL_CALL(ColorMask, 1, 1, 1, 1);
  if (has_clip) {
    GL_CALL(StencilMask, 0x1F);
    GL_CALL(StencilFunc, GL_EQUAL, 0x10, 0x1F);
    GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_KEEP);
  } else {
    GL_CALL(StencilMask, 0x0F);
    GL_CALL(StencilFunc, GL_EQUAL, 0x00, 0x0F);
    GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_KEEP);
  }
  GL_CALL(StencilMask, 0x0F);
}

void GLDrawMeshOpAA::DrawAAOutline() {
  mesh_->BindAAOutlineIndex();

  GLMeshDraw{GL_TRIANGLES, aa_start_, aa_count_}();
}

}  // namespace skity