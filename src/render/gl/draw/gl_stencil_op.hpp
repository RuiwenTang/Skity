#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_STENCIL_OP_H
#define SKITY_SRC_RENDER_GL_DRAW_GL_STENCIL_OP_H

#include "src/render/gl/draw/gl_draw_mesh_op.hpp"

namespace skity {

class StencilShader;

class GLStencilDrawOp : public GLDrawMeshOp {
 public:
  GLStencilDrawOp(uint32_t front_start, uint32_t front_count,
                  uint32_t back_start, uint32_t back_count,
                  StencilShader* shader, GLMesh* mesh, bool positive = true);
  ~GLStencilDrawOp() override = default;

  void SetStencilValue(uint32_t value) { stencil_value_ = value; }

  void SetStencilMask(uint32_t mask) { stencil_mask_ = mask; }

  void SetStencilFrontFlag(uint32_t flag) { front_flag_ = flag; }

  void SetStencilBackFlag(uint32_t flag) { back_flag_ = flag; }

 protected:
  void OnBeforeDraw() override;
  void OnAfterDraw() override;
  void OnBeforeDrawFront() override;
  void OnBeforeDrawBack() override;
  void UpdateStrokeWidth(float width);

  void UpdateStencilValues();

 private:
  StencilShader* shader_;
  bool postive_ = true;
  uint32_t stencil_value_;
  uint32_t stencil_mask_;
  uint32_t front_flag_;
  uint32_t back_flag_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_STENCIL_OP_H