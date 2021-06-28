#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H
#define SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H

#include "src/render/gl/gl_draw_op.hpp"

namespace skity {

class GLShader;
class GLMesh;

class GLDrawMeshOp : public GLDrawOp {
 public:
  GLDrawMeshOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
               uint32_t back_count, GLShader* shader, GLMesh* mesh);

  void SetStencilValue(uint32_t value) { stencil_value_ = value; }

  void SetStencilMask(uint32_t mask) { stencil_mask_ = mask; }

  void SetStencilFrontFlag(uint32_t flag) { front_flag_ = flag; }

  void SetStencilBackFlag(uint32_t flag) { back_flag_ = flag; }

 protected:
  void OnBeforeDraw() override;
  void OnAfterDraw() override;
  void OnDraw() override;
  void OnInit() override;

 private:
  void OnBeforeDrawFront();
  void OnBeforeDrawBack();
  void DrawFront();
  void DrawBack();

 private:
  uint32_t stencil_value_;
  uint32_t stencil_mask_;
  uint32_t front_flag_;
  uint32_t back_flag_;
  GLShader* shader_;
  GLMesh* mesh_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H