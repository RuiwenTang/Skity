#ifndef SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
#define SKITY_SRC_RENDER_GL_GL_DRAW_OP_H

#include <memory>

namespace skity {

class StencilShader;

class GLDrawOp {
 public:
  GLDrawOp() = default;
  virtual ~GLDrawOp() = default;

  virtual void Draw() = 0;
};

class GLDrawStencilOp : public GLDrawOp {
 public:
  GLDrawStencilOp(uint32_t front_start, uint32_t front_count,
                  uint32_t back_start, uint32_t back_count,
                  StencilShader* stencil_shader);
  ~GLDrawStencilOp() override = default;

  void Draw() override;

 protected:
  void DrawFront();
  void DrawBack();
  void DoStencil();
  virtual void DoDraw() = 0;

 private:
  uint32_t front_start_ = 0;
  uint32_t front_count_ = 0;
  uint32_t back_start_ = 0;
  uint32_t back_count_ = 0;
  StencilShader* stencil_shader_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP_H