#include "src/render/gl/gl_canvas.hpp"

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "src/render/gl/gl_fill.hpp"
#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_stroke.hpp"
#include "src/render/gl/gl_stroke_aa.hpp"

namespace skity {

class GLCanvasStateOp : public GLDrawOp {
 public:
  GLCanvasStateOp(GLCanvasState* state)
      : GLDrawOp(0, 0, 0, 0, nullptr), state_(state) {}

  ~GLCanvasStateOp() override = default;

 protected:
  void OnDraw(bool has_clip) override {}

  void OnInit() override {}

 protected:
  GLCanvasState* state_;
};

class GLCanvasClipOp : public GLCanvasStateOp {
 public:
  GLCanvasClipOp(GLCanvasState* state, GLMeshRange const& range)
      : GLCanvasStateOp(state), range_(range) {}
  ~GLCanvasClipOp() override = default;

 protected:
  void OnDraw(bool has_clip) override {
    int32_t depth = state_->CurrentStackDepth();
    state_->UpdateCurrentClipPathRange(range_);
    state_->DoClipPath(depth);
  }

 private:
  GLMeshRange range_;
};

class GLCanvasSaveOp : public GLCanvasStateOp {
 public:
  GLCanvasSaveOp(GLCanvasState* state) : GLCanvasStateOp(state) {}
  ~GLCanvasSaveOp() override = default;

 protected:
  void OnDraw(bool has_clip) override { state_->PushStack(); }
};

class GLCanvasRestoreOp : public GLCanvasStateOp {
 public:
  GLCanvasRestoreOp(GLCanvasState* state) : GLCanvasStateOp(state) {}
  ~GLCanvasRestoreOp() override = default;

 protected:
  void OnDraw(bool has_clip) override {
    state_->UnDoClipPath();
    bool need_do_clip = state_->CurrentHasClipPath();
    state_->PopStack();
    if (need_do_clip) {
      state_->DoClipPath(state_->CurrentStackDepth());
    }
  }
};

class GLCanvasTranslateOp : public GLCanvasStateOp {
 public:
  GLCanvasTranslateOp(GLCanvasState* state, float dx, float dy)
      : GLCanvasStateOp(state), dx_(dx), dy_(dy) {}

  ~GLCanvasTranslateOp() override = default;

 protected:
  void OnDraw(bool has_clip) override {
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
  void OnDraw(bool has_clip) override {
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
  void OnDraw(bool has_clip) override {
    auto current_matrix = state_->CurrentMatrix();
    auto rotate_matrix = glm::rotate(glm::identity<glm::mat4>(),
                                     glm::radians(degree_), {0.f, 0.f, 1.f});

    auto final_matrix = rotate_matrix * current_matrix;
    state_->UpdateCurrentMatrix(final_matrix);
  }

 private:
  float degree_;
};

class GLCanvasRotateWithPointOp : public GLCanvasStateOp {
 public:
  GLCanvasRotateWithPointOp(GLCanvasState* state, float degree, float x,
                            float y)
      : GLCanvasStateOp(state), degree_(degree), x_(x), y_(y) {}
  ~GLCanvasRotateWithPointOp() override = default;

 protected:
  void OnDraw(bool has_clip) override {
    auto current_matrix = state_->CurrentMatrix();
    auto prev_translate_matrix =
        glm::translate(glm::identity<glm::mat4>(), glm::vec3(-x_, -y_, 0.f));
    auto rotate_matrix = glm::rotate(glm::identity<glm::mat4>(),
                                     glm::radians(degree_), {0.f, 0.f, 1.f});
    auto post_translate_matrix =
        glm::translate(glm::identity<glm::mat4>(), glm::vec3(x_, y_, 0.f));

    auto final_matrix = post_translate_matrix * rotate_matrix *
                        prev_translate_matrix * current_matrix;

    state_->UpdateCurrentMatrix(final_matrix);
  }

 private:
  float degree_;
  float x_;
  float y_;
};

std::unique_ptr<Canvas> Canvas::MakeGLCanvas(uint32_t x, uint8_t y,
                                             uint32_t width, uint32_t height,
                                             void* procss_loader) {
  GLInterface::InitGlobalInterface(procss_loader);
  Matrix mvp = glm::ortho<float>(x, x + width, y + height, y);

  return std::make_unique<GLCanvas>(mvp);
}

GLCanvasState::GLCanvasState(Matrix mvp, GLMesh* mesh, StencilShader* shader,
                             ColorShader* color_shader)
    : mvp_(mvp), mesh_(mesh), shader_(shader), color_shader_(color_shader) {
  State init_state;
  init_state.matrix = glm::identity<glm::mat4>();

  state_stack_.emplace_back(init_state);
  clip_stack_.emplace_back(false);
}

void GLCanvasState::UpdateCurrentMatrix(Matrix const& matrix) {
  assert(!state_stack_.empty());
  state_stack_.back().matrix = matrix;
}

void GLCanvasState::UpdateCurrentClipPathRange(GLMeshRange const& range) {
  assert(!state_stack_.empty());
  state_stack_.back().clip_path_range = range;
  state_stack_.back().has_clip = true;
  clip_stack_.back() = true;
}

Matrix GLCanvasState::CurrentMatrix() {
  assert(!state_stack_.empty());
  return state_stack_.back().matrix;
}

void GLCanvasState::DoClipPath(uint32_t stack_depth) {
  int32_t clip_index = stack_depth - 1;
  for (; clip_index >= 0; clip_index--) {
    if (state_stack_[clip_index].has_clip) {
      break;
    }
  }

  if (clip_index < 0) {
    return;
  }
  // FIXME: make sure no color is output
  GL_CALL(ColorMask, 0, 0, 0, 0);
  State const& state = state_stack_[clip_index];
  Matrix final_matrix = mvp_ * state.matrix;
  shader_->Bind();
  shader_->SetMVPMatrix(final_matrix);
  // step 2 stencil path as fill path do
  GL_CALL(StencilMask, 0x0F);
  GL_CALL(StencilFunc, GL_ALWAYS, 0x01, 0x0F);

  mesh_->BindMesh();
  // front
  mesh_->BindFrontIndex();
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
  DrawFront(state.clip_path_range);
  // back
  mesh_->BindBackIndex();
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
  DrawBack(state.clip_path_range);

  GL_CALL(StencilMask, 0x1F);
  GL_CALL(StencilFunc, GL_NOTEQUAL, 0x10, 0x0F);
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);
  // front
  mesh_->BindFrontIndex();
  DrawFront(state.clip_path_range);
  // back
  mesh_->BindBackIndex();
  DrawBack(state.clip_path_range);
  // reset stencil mask
  GL_CALL(StencilMask, 0x0F);
}

void GLCanvasState::UnDoClipPath() {
  if (state_stack_.empty() || !state_stack_.back().has_clip) {
    return;
  }
  auto const& state = state_stack_.back();
  Matrix final_matrix = mvp_ * state.matrix;

  shader_->Bind();
  shader_->SetMVPMatrix(final_matrix);

  GL_CALL(StencilMask, 0x1F);
  GL_CALL(StencilFunc, GL_ALWAYS, 0x00, 0x1F);
  GL_CALL(ColorMask, 0, 0, 0, 0);
  mesh_->BindMesh();
  // front
  mesh_->BindFrontIndex();
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);
  DrawFront(state.clip_path_range);
  // back
  mesh_->BindBackIndex();
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);
  DrawBack(state.clip_path_range);

  GL_CALL(StencilMask, 0x0F);
  GL_CALL(ColorMask, 1, 1, 1, 1);
}

void GLCanvasState::DrawFront(GLMeshRange const& range) {
  GLMeshDraw{GL_TRIANGLES, range.front_start, range.front_count}();
}

void GLCanvasState::DrawBack(GLMeshRange const& range) {
  GLMeshDraw{GL_TRIANGLES, range.back_start, range.back_count}();
}

int32_t GLCanvasState::CurrentStackDepth() const { return state_stack_.size(); }

void GLCanvasState::PushStack() {
  State state;
  state.matrix = CurrentMatrix();
  state_stack_.push_back(state);
  clip_stack_.emplace_back(clip_stack_.back());
}

void GLCanvasState::PopStack(int32_t target_stack_depth) {
  state_stack_.erase(state_stack_.begin() + target_stack_depth,
                     state_stack_.end());
  clip_stack_.erase(clip_stack_.begin() + target_stack_depth,
                    clip_stack_.end());
}

void GLCanvasState::PopStack() {
  if (state_stack_.size() <= 1) {
    return;
  }

  state_stack_.pop_back();
  clip_stack_.pop_back();
}

bool GLCanvasState::HasClip() { return clip_stack_.back(); }

bool GLCanvasState::CurrentHasClipPath() {
  return state_stack_.back().has_clip;
}

GLCanvas::GLCanvas(Matrix const& mvp) : Canvas(), mvp_(mvp) { Init(); }

void GLCanvas::Init() {
  InitShader();
  InitMesh();
  InitDrawOpBuilder();
  gl_vertex_.Reset();
  // clear stencil buffer
  GL_CALL(ClearStencil, 0x0);
  // Init state
  state_ = std::make_unique<GLCanvasState>(
      mvp_, mesh_.get(), stencil_shader_.get(), color_shader_.get());
}

void GLCanvas::InitShader() {
  stencil_shader_ = GLShader::CreateStencilShader();
  color_shader_ = GLShader::CreateColorShader();
  gradient_shader_ = GLShader::CreateGradientShader();
}

void GLCanvas::InitMesh() {
  vertex_ = std::make_unique<GLVertex>();
  mesh_ = std::make_unique<GLMesh>();
  mesh_->Init();
}

void GLCanvas::InitDrawOpBuilder() {
  draw_op_builder_.UpdateStencilShader(stencil_shader_.get());
  draw_op_builder_.UpdateColorShader(color_shader_.get());
  draw_op_builder_.UpdateGradientShader(gradient_shader_.get());
  draw_op_builder_.UpdateMesh(mesh_.get());
}

void GLCanvas::onClipPath(Path const& path, ClipOp op) {
  GLFill gl_fill{};
  skity::Paint paint;
  GLMeshRange clip_range = gl_fill.fillPath(path, paint, &gl_vertex_);

  draw_ops_.emplace_back(
      std::make_unique<GLCanvasClipOp>(state_.get(), clip_range));
}

void GLCanvas::UpdateDrawOpBuilder(GLMeshRange const& range) {
  draw_op_builder_.UpdateFrontStart(range.front_start);
  draw_op_builder_.UpdateFrontCount(range.front_count);
  draw_op_builder_.UpdateBackStart(range.back_start);
  draw_op_builder_.UpdateBackCount(range.back_count);
}

std::unique_ptr<GLDrawOp> GLCanvas::GenerateColorOp(Paint const& paint,
                                                    bool fill,
                                                    GLMeshRange* aa_range) {
  uint32_t aa_outline_start = 0;
  uint32_t aa_outline_count = 0;
  if (aa_range) {
    aa_outline_start = aa_range->aa_outline_start;
    aa_outline_count = aa_range->aa_outline_count;
  }

  if (paint.getShader()) {
    Shader::GradientInfo gradient_info{};
    Shader::GradientType type = paint.getShader()->asGradient(&gradient_info);

    if (type != Shader::kNone) {
      return draw_op_builder_.CreateGradientOpAA(
          &gradient_info, type, aa_outline_start, aa_outline_count);
    } else {
      // TODO implement Other shader type
      return nullptr;
    }
  } else {
    auto color = fill ? paint.GetFillColor() : paint.GetStrokeColor();
    return draw_op_builder_.CreateColorOpAA(color.r, color.g, color.b, color.a,
                                            aa_outline_start, aa_outline_count);
  }
}

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

    bool need_antialias = paint.isAntiAlias();

    if (need_stroke && need_antialias) {
      need_antialias = false;
      if (paint.getPathEffect() && paint.getPathEffect()->asADash(nullptr) ==
                                       PathEffect::DashType::kDash) {
        need_antialias = true;
      }
    }

    GLMeshRange aa_range{};
    if (need_antialias) {
      GLStrokeAA strokeAA(1.f);
      aa_range = strokeAA.StrokePathAA(path, &gl_vertex_);
    }
    auto op = GenerateColorOp(paint, true, &aa_range);
    if (op) {
      draw_ops_.emplace_back(std::move(op));
    }

    // clear current stencil value
    draw_ops_.emplace_back(
        std::move(draw_op_builder_.CreateStencilOp(0, false)));

    if (isDrawDebugLine()) {
      draw_ops_.emplace_back(std::move(draw_op_builder_.CreateDebugLineOp()));
    }
  }

  if (need_stroke) {
    GLStroke gl_stroke(paint);
    GLMeshRange stroke_range = gl_stroke.strokePath(path, &gl_vertex_);

    UpdateDrawOpBuilder(stroke_range);
    draw_ops_.emplace_back(
        std::move(draw_op_builder_.CreateStencilOp(paint.getStrokeWidth())));

    auto stroke_color = paint.GetStrokeColor();

    GLMeshRange aa_range{};
    aa_range.aa_outline_start = stroke_range.aa_outline_start;
    aa_range.aa_outline_count = stroke_range.aa_outline_count;

    auto op = GenerateColorOp(paint, false, &aa_range);
    if (op) {
      draw_ops_.emplace_back(std::move(op));
    }
    // clear stencil value
    draw_ops_.emplace_back(std::move(
        draw_op_builder_.CreateStencilOp(paint.getStrokeWidth(), false)));

    if (isDrawDebugLine()) {
      draw_ops_.emplace_back(std::move(draw_op_builder_.CreateDebugLineOp()));
    }
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

  mesh_->uploadAaOutlineIndex(gl_vertex_.GetAAIndexData(),
                              gl_vertex_.GetAAIndexDataSize());

  GL_CALL(ColorMask, 1, 1, 1, 1);
  GL_CALL(Clear, GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  GL_CALL(ColorMask, 0, 0, 0, 0);
  for (auto const& op : draw_ops_) {
    Matrix mvp = mvp_ * state_->CurrentMatrix();
    op->UpdateCurrentMatrix(state_->CurrentMatrix());
    op->Draw(mvp, state_->HasClip());
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

void GLCanvas::onRotate(float degree, float px, float py) {
  draw_ops_.emplace_back(std::make_unique<GLCanvasRotateWithPointOp>(
      state_.get(), degree, px, py));
}

void GLCanvas::onUpdateViewport(uint32_t width, uint32_t height) {
  mvp_ = glm::ortho<float>(0, width, height, 0);
}

}  // namespace skity