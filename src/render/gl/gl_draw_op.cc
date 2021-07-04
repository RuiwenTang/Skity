#include "src/render/gl/gl_draw_op.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "src/render/gl/draw/gl_clear_stencil_op.hpp"
#include "src/render/gl/draw/gl_fill_color_op.hpp"
#include "src/render/gl/draw/gl_stencil_op.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

void GLDrawOp::Draw() {
  OnBeforeDraw();
  OnDraw();
  OnAfterDraw();
}

void GLDrawOp::Init() { OnInit(); }

void GLDrawOpBuilder::UpdateStencilShader(StencilShader* shader) {
  stencil_shader = shader;
}

void GLDrawOpBuilder::UpdateColorShader(ColorShader* shader) {
  color_shader = shader;
}

void GLDrawOpBuilder::UpdateMesh(GLMesh* mesh) { gl_mesh = mesh; }

void GLDrawOpBuilder::UpdateMVPMatrix(glm::mat4 const& matrix) {
  mvp_matrix = matrix;
}

void GLDrawOpBuilder::UpdateFrontStart(uint32_t value) { front_start = value; }

void GLDrawOpBuilder::UpdateFrontCount(uint32_t value) { front_count = value; }

void GLDrawOpBuilder::UpdateBackStart(uint32_t value) { back_start = value; }

void GLDrawOpBuilder::UpdateBackCount(uint32_t value) { back_count = value; }

std::unique_ptr<GLDrawOp> GLDrawOpBuilder::CreateStencilOp(float stroke_width,
                                                           bool positive) {
  auto op = std::make_unique<GLStencilDrawOp>(
      front_start, front_count, back_start, back_count, stencil_shader, gl_mesh,
      positive);

  op->SetMVPMatrix(mvp_matrix);
  op->SetStrokeWidth(stroke_width);

  return op;
}

std::unique_ptr<GLDrawOp> GLDrawOpBuilder::CreateColorOp(float r, float g,
                                                         float b, float a) {
  auto op = std::make_unique<GLFillColorOp>(
      front_start, front_count, back_start, back_count, color_shader, gl_mesh);

  op->SetMVPMatrix(mvp_matrix);
  op->SetColor(r, g, b, a);

  return op;
}

std::unique_ptr<GLDrawOp> GLDrawOpBuilder::CreateClearStencilOp() {
  return std::make_unique<GLClearStencilOp>();
}

}  // namespace skity
