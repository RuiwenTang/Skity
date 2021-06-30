#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_FILL_COLOR_OP_H
#define SKITY_SRC_RENDER_GL_DRAW_GL_FILL_COLOR_OP_H

#include "src/render/gl/draw/gl_draw_mesh_op.hpp"

namespace skity {

class ColorShader;

class GLFillColorOp : public GLDrawMeshOp {
 public:
  GLFillColorOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
                uint32_t back_count, ColorShader* shader, GLMesh* mesh);

  ~GLFillColorOp() override = default;

  void SetColor(float r, float g, float b, float a);

 protected:
  void OnBeforeDraw() override;
  void OnAfterDraw() override;

 private:
  ColorShader* shader_;
  float r_ = 0.f;
  float g_ = 0.f;
  float b_ = 0.f;
  float a_ = 1.f;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_FILL_COLOR_OP_H