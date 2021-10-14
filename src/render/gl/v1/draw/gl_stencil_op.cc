#include "gl_stencil_op.hpp"

#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLStencilDrawOp::GLStencilDrawOp(uint32_t front_start, uint32_t front_count,
                                 uint32_t back_start, uint32_t back_count,
                                 StencilShader* shader, GLMesh* mesh,
                                 bool positive)
    : GLDrawMeshOp(front_start, front_count, back_start, back_count, shader,
                   mesh),
      shader_(shader),
      postive_(positive),
      stencil_value_(0x01),
      stencil_mask_(0x0F),
      front_flag_(GL_INCR_WRAP),
      back_flag_(GL_DECR_WRAP) {
  UpdateStencilValues();
}

void GLStencilDrawOp::OnBeforeDraw(bool has_clip) {
  GLDrawMeshOp::OnBeforeDraw(has_clip);
  GL_CALL(ColorMask, 0, 0, 0, 0);
  GL_CALL(Enable, GL_STENCIL_TEST);
  GL_CALL(StencilMask, stencil_mask_);
  GL_CALL(StencilFunc, GL_ALWAYS, 0x01, stencil_mask_);

  if (stroke_width_ > 0.f) {
    UpdateStrokeWidth(stroke_width_);
  }
}

void GLStencilDrawOp::OnAfterDraw(bool has_clip) {
  GLDrawMeshOp::OnAfterDraw(has_clip);
  GL_CALL(ColorMask, 1, 1, 1, 1);
}

void GLStencilDrawOp::OnBeforeDrawFront() {
  GLDrawMeshOp::OnBeforeDrawFront();
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, front_flag_);
}

void GLStencilDrawOp::OnBeforeDrawBack() {
  GLDrawMeshOp::OnBeforeDrawBack();
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, back_flag_);
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