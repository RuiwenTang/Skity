#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_CLEAR_STENCIL_OP_HPP
#define SKITY_SRC_RENDER_GL_DRAW_GL_CLEAR_STENCIL_OP_HPP

#include "src/render/gl/gl_draw_op.hpp"

namespace skity {

class GLClearStencilOp : public GLDrawOp {
 public:
  GLClearStencilOp();
  ~GLClearStencilOp() override = default;

 protected:
  void OnBeforeDraw(bool has_clip) override;
  void OnAfterDraw(bool has_clip) override;
  void OnDraw(bool has_clip) override;
  void OnInit() override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_CLEAR_STENCIL_OP_HPP