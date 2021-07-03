#include "src/render/gl/gl_canvas.hpp"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include "src/render/gl/gl_fill.hpp"
#include "src/render/gl/gl_stroke.hpp"

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeGLCanvas(uint32_t x, uint8_t y,
                                             uint32_t width, uint32_t height) {
  Matrix mvp = glm::ortho<float>(x, x + width, y + height, y);

  return std::make_unique<GLCanvas>(mvp);
}

GLCanvas::GLCanvas(Matrix const& mvp) : Canvas(), mvp_(mvp) { Init(); }

void GLCanvas::Init() {
  InitShader();
  InitMesh();
  InitDrawOpBuilder();
  gl_vertex_.Reset();
  // clear stencil buffer
  glClearStencil(0x0);
}

void GLCanvas::InitShader() {
  stencil_shader_ = GLShader::CreateStencilShader();
  color_shader_ = GLShader::CreateColorShader();
}

void GLCanvas::InitMesh() {
  vertex_ = std::make_unique<GLVertex>();
  mesh_ = std::make_unique<GLMesh>();
  mesh_->Init();
}

void GLCanvas::InitDrawOpBuilder() {
  skity::GLDrawOpBuilder::UpdateStencilShader(stencil_shader_.get());
  skity::GLDrawOpBuilder::UpdateColorShader(color_shader_.get());
  skity::GLDrawOpBuilder::UpdateMesh(mesh_.get());
  skity::GLDrawOpBuilder::UpdateMVPMatrix(mvp_);
}

void GLCanvas::onClipPath(Path const& path, ClipOp op) {}

void GLCanvas::UpdateDrawOpBuilder(GLMeshRange const& range) {
  skity::GLDrawOpBuilder::UpdateFrontStart(range.front_start);
  skity::GLDrawOpBuilder::UpdateFrontCount(range.front_count);
  skity::GLDrawOpBuilder::UpdateBackStart(range.back_start);
  skity::GLDrawOpBuilder::UpdateBackCount(range.back_count);
}

void onClipPath(Path const& path, Canvas::ClipOp op) {}

void GLCanvas::onDrawPath(Path const& path, Paint const& paint) {
  bool need_fill = false;
  bool need_stroke = false;
  auto style = paint.getStyle();

  switch (style) {
    case Paint::kFill_Style:
      need_fill = true;
      break;
    case Paint::kStroke_Style:
      need_stroke = true;
      break;
    case Paint::kStrokeAndFill_Style:
      need_fill = need_stroke = true;
      break;
  }

  // TODO handle clip
  if (need_fill) {
    draw_ops_.emplace_back(
        std::move(skity::GLDrawOpBuilder::CreateClearStencilOp()));

    GLFill gl_fill{};
    GLMeshRange fill_range = gl_fill.fillPath(path, paint, &gl_vertex_);

    UpdateDrawOpBuilder(fill_range);
    draw_ops_.emplace_back(
        std::move(skity::GLDrawOpBuilder::CreateStencilOp()));

    auto fill_color = paint.GetFillColor();

    draw_ops_.emplace_back(std::move(skity::GLDrawOpBuilder::CreateColorOp(
        fill_color[0], fill_color[1], fill_color[2], fill_color[3])));
  }

  if (need_stroke) {
    draw_ops_.emplace_back(
        std::move(skity::GLDrawOpBuilder::CreateClearStencilOp()));

    GLStroke gl_stroke(paint);
    GLMeshRange stroke_range = gl_stroke.strokePath(path, &gl_vertex_);

    UpdateDrawOpBuilder(stroke_range);
    draw_ops_.emplace_back(std::move(
        skity::GLDrawOpBuilder::CreateStencilOp(paint.getStrokeWidth())));

    auto stroke_color = paint.GetStrokeColor();

    draw_ops_.emplace_back(std::move(skity::GLDrawOpBuilder::CreateColorOp(
        stroke_color[0], stroke_color[1], stroke_color[2], stroke_color[3])));
  }
}

void GLCanvas::onFlush() {
  mesh_->BindMesh();
  mesh_->UploadVertexBuffer(gl_vertex_.GetVertexData(),
                            gl_vertex_.GetVertexDataSize());

  mesh_->UploadFrontIndex(gl_vertex_.GetFrontIndexData(),
                          gl_vertex_.GetFrontIndexDataSize());

  mesh_->UploadBackIndex(gl_vertex_.GetBackIndexData(),
                         gl_vertex_.GetBackIndexDataSize());

  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  for (auto const& op : draw_ops_) {
    op->Draw();
  }

  draw_ops_.clear();

  gl_vertex_.Reset();
}

void GLCanvas::onSave() {}

void GLCanvas::onRestore() {}

}  // namespace skity