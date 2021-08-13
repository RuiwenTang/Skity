#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_FILL_GRADIENT_OP_H
#define SKITY_SRC_RENDER_GL_DRAW_GL_FILL_GRADIENT_OP_H

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/effect/shader.hpp>
#include <skity/geometry/point.hpp>
#include <vector>

#include "src/render/gl/draw/gl_draw_mesh_op_aa.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

class GLFillGradientOp : public GLDrawMeshOpAA {
 public:
  GLFillGradientOp(uint32_t front_start, uint32_t front_count,
                   uint32_t back_start, uint32_t back_count,
                   GLGradientShader* shader, GLMesh* mesh)
      : GLDrawMeshOpAA(front_start, front_count, back_start, back_count, 0, 0,
                       shader, mesh),
        shader_(shader) {}

  GLFillGradientOp(uint32_t front_start, uint32_t front_count,
                   uint32_t back_start, uint32_t back_count, uint32_t aa_start,
                   uint32_t aa_count, GLGradientShader* shader, GLMesh* mesh)
      : GLDrawMeshOpAA(front_start, front_count, back_start, back_count,
                       aa_start, aa_count, shader, mesh),
        shader_(shader) {}

  ~GLFillGradientOp() override = default;

  void SetPoints(Point const& p1, Point const& p2);
  void SetRadius(float r1, float r2);
  void SetColors(std::vector<Vec4> const& colors);
  void SetStops(std::vector<float> const& stops);
  void SetGradientType(Shader::GradientType type);
  void SetLocalMatrix(Matrix const& matrix);
  void SetGradientFlag(int32_t flag);

 protected:
  void OnBeforeDraw(bool has_clip) override;

 private:
  std::array<Point, 2> points_ = {};
  std::array<float, 2> radius_ = {};
  std::vector<Vec4> colors_ = {};
  std::vector<float> stops_ = {};
  Shader::GradientType type_;
  Matrix local_matrix_ = glm::identity<glm::mat4>();
  int32_t gradient_flag_ = 0;
  GLGradientShader* shader_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_FILL_GRADIENT_OP_H