#include "src/render/gl/gl_canvas.hpp"

#include <glad/glad.h>

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

#include "src/render/gl/gl_fill.hpp"
#include "src/render/gl/gl_stroke.hpp"

namespace skity {

class GLCanvasStateOp : public GLDrawOp {
 public:
  GLCanvasStateOp(GLCanvasState* state)
      : GLDrawOp(0, 0, 0, 0, nullptr), state_(state) {}

  ~GLCanvasStateOp() override = default;

 protected:
  void OnDraw() override {}

  void OnInit() override {}

 protected:
  GLCanvasState* state_;
};

class GLCanvasSaveOp : public GLCanvasStateOp {
 public:
  GLCanvasSaveOp(GLCanvasState* state) : GLCanvasStateOp(state) {}
  ~GLCanvasSaveOp() override = default;

 protected:
  void OnDraw() override { state_->PushStack(); }
};

class GLCanvasRestoreOp : public GLCanvasStateOp {
 public:
  GLCanvasRestoreOp(GLCanvasState* state) : GLCanvasStateOp(state) {}
  ~GLCanvasRestoreOp() override = default;

 protected:
  void OnDraw() override { state_->PopStack(); }
};

class GLCanvasTranslateOp : public GLCanvasStateOp {
 public:
  GLCanvasTranslateOp(GLCanvasState* state, float dx, float dy)
      : GLCanvasStateOp(state), dx_(dx), dy_(dy) {}

  ~GLCanvasTranslateOp() override = default;

 protected:
  void OnDraw() override {
    auto current_matrix = state_->CurrentMatrix();
    auto translate_matrix =
        glm::translate(glm::identity<glm::mat4>(), {dx_, dy_, 0.f});

    auto final_matrix = translate_matrix * current_matrix;
    state_->UpdateCurrentMatrix(final_matrix);
  }

 private:
  float dx_;
  float dy_;
};

class GLCanvasScaleOp : public GLCanvasStateOp {
 public:
  GLCanvasScaleOp(GLCanvasState* state, float sx, float sy)
      : GLCanvasStateOp(state), sx_(sx), sy_(sy) {}

  ~GLCanvasScaleOp() override = default;

 protected:
  void OnDraw() override {
    auto current_matrix = state_->CurrentMatrix();
    auto scale_matrix = glm::scale(glm::identity<glm::mat4>(), {sx_, sy_, 1.f});

    auto final_matrix = scale_matrix * current_matrix;
    state_->UpdateCurrentMatrix(final_matrix);
  }

 private:
  float sx_;
  float sy_;
};

class GLCanvasRotateOp : public GLCanvasStateOp {
 public:
  GLCanvasRotateOp(GLCanvasState* state, float degree)
      : GLCanvasStateOp(state), degree_(degree) {}

  ~GLCanvasRotateOp() override = default;

 protected:
  void OnDraw() override {
    auto current_matrix = state_->CurrentMatrix();
    auto rotate_matrix = glm::rotate(glm::identity<glm::mat4>(),
                                     glm::radians(degree_), {0.f, 0.f, 1.f});

    auto final_matrix = rotate_matrix * current_matrix;
    state_->UpdateCurrentMatrix(final_matrix);
  }

 private:
  float degree_;
};

std::unique_ptr<Canvas> Canvas::MakeGLCanvas(uint32_t x, uint8_t y,
                                             uint32_t width, uint32_t height) {
  Matrix mvp = glm::ortho<float>(x, x + width, y + height, y);

  return std::make_unique<GLCanvas>(mvp);
}

GLCanvasState::GLCanvasState() {
  State init_state;
  init_state.matrix = glm::identity<glm::mat4>();

  state_stack_.emplace_back(init_state);
}

void GLCanvasState::UpdateCurrentMatrix(Matrix const& matrix) {
  assert(!state_stack_.empty());
  state_stack_.back().matrix = matrix;
}

void GLCanvasState::UpdateCurrentClipPathRange(GLMeshRange const& range) {
  assert(!state_stack_.empty());
  state_stack_.back().clip_path_range = range;
  state_stack_.back().has_clip = true;
}

Matrix GLCanvasState::CurrentMatrix() {
  assert(!state_stack_.empty());
  return state_stack_.back().matrix;
}

void DoClipPath(uint32_t stack_depth) {}

int32_t GLCanvasState::CurrentStackDepth() const { return state_stack_.size(); }

void GLCanvasState::PushStack() {
  State state;
  state.matrix = CurrentMatrix();
  state_stack_.push_back(state);
}

void GLCanvasState::PopStack(int32_t target_stack_depth) {
  state_stack_.erase(state_stack_.begin() + target_stack_depth,
                     state_stack_.end());
}

void GLCanvasState::PopStack() {
  if (state_stack_.size() <= 1) {
    return;
  }

  state_stack_.pop_back();
}

GLCanvas::GLCanvas(Matrix const& mvp) : Canvas(), mvp_(mvp) {
  state_ = std::make_unique<GLCanvasState>();
  Init();
}

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
  draw_op_builder_.UpdateStencilShader(stencil_shader_.get());
  draw_op_builder_.UpdateColorShader(color_shader_.get());
  draw_op_builder_.UpdateMesh(mesh_.get());
}

void GLCanvas::onClipPath(Path const& path, ClipOp op) {}

void GLCanvas::UpdateDrawOpBuilder(GLMeshRange const& range) {
  draw_op_builder_.UpdateFrontStart(range.front_start);
  draw_op_builder_.UpdateFrontCount(range.front_count);
  draw_op_builder_.UpdateBackStart(range.back_start);
  draw_op_builder_.UpdateBackCount(range.back_count);
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
    GLFill gl_fill{};
    GLMeshRange fill_range = gl_fill.fillPath(path, paint, &gl_vertex_);

    UpdateDrawOpBuilder(fill_range);
    draw_ops_.emplace_back(std::move(draw_op_builder_.CreateStencilOp()));

    auto fill_color = paint.GetFillColor();

    draw_ops_.emplace_back(std::move(draw_op_builder_.CreateColorOp(
        fill_color[0], fill_color[1], fill_color[2], fill_color[3])));

    // clear current stencil value
    draw_ops_.emplace_back(
        std::move(draw_op_builder_.CreateStencilOp(0, false)));
  }

  if (need_stroke) {
    GLStroke gl_stroke(paint);
    GLMeshRange stroke_range = gl_stroke.strokePath(path, &gl_vertex_);

    UpdateDrawOpBuilder(stroke_range);
    draw_ops_.emplace_back(
        std::move(draw_op_builder_.CreateStencilOp(paint.getStrokeWidth())));

    auto stroke_color = paint.GetStrokeColor();

    draw_ops_.emplace_back(std::move(draw_op_builder_.CreateColorOp(
        stroke_color[0], stroke_color[1], stroke_color[2], stroke_color[3])));

    // clear stencil value
    draw_ops_.emplace_back(std::move(
        draw_op_builder_.CreateStencilOp(paint.getStrokeWidth(), false)));
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
    Matrix mvp = mvp_ * state_->CurrentMatrix();
    op->Draw(mvp);
  }

  draw_ops_.clear();

  gl_vertex_.Reset();
}

void GLCanvas::onSave() {
  draw_ops_.emplace_back(std::make_unique<GLCanvasSaveOp>(state_.get()));
}

void GLCanvas::onRestore() {
  draw_ops_.emplace_back(std::make_unique<GLCanvasRestoreOp>(state_.get()));
}

void GLCanvas::onTranslate(float dx, float dy) {
  draw_ops_.emplace_back(
      std::make_unique<GLCanvasTranslateOp>(state_.get(), dx, dy));
}

void GLCanvas::onScale(float sx, float sy) {
  draw_ops_.emplace_back(
      std::make_unique<GLCanvasScaleOp>(state_.get(), sx, sy));
}

void GLCanvas::onRotate(float degree) {
  draw_ops_.emplace_back(
      std::make_unique<GLCanvasRotateOp>(state_.get(), degree));
}

void GLCanvas::onRotate(float degree, float px, float py) {}

}  // namespace skity