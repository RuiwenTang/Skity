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

 protected:
  void UpdateStrokeWidth(float width);

  void UpdateStencilValues();

 private:
  StencilShader* shader_;
  bool postive_ = true;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_STENCIL_OP_H