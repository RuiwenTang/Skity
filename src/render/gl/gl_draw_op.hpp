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
    StencilShader* GetStencilShader() { return stencil_shader_; }
  void DrawFront();
  void DrawBack();
  void DoStencil();
    void DoDraw();
  // sub class override this to update shader uniforms
  virtual void OnBeforeDoStencil() = 0;
  // sub class override this to bind custom shader for color
  virtual void OnBeforeDoDraw() = 0;

 private:
  uint32_t front_start_ = 0;
  uint32_t front_count_ = 0;
  uint32_t back_start_ = 0;
  uint32_t back_count_ = 0;
  StencilShader* stencil_shader_;
};

class FillStencilDrawOp: public GLDrawStencilOp {
public:
    FillStencilDrawOp(uint32_t front_start, uint32_t front_count,
                      uint32_t back_start, uint32_t back_count,
                      StencilShader* stencil_shader): GLDrawStencilOp(front_start, front_count, back_start, back_count, stencil_shader) {}

protected:
    void OnBeforeDoStencil() override;
    void OnBeforeDoDraw() override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
