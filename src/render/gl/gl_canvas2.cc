
#include "src/render/gl/gl_canvas2.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/effect/path_effect.hpp>
#include <unordered_map>

#include "src/render/gl/gl_draw_op2.hpp"
#include "src/render/gl/gl_fill2.hpp"
#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_stroke2.hpp"
#include "src/render/gl/gl_texture.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeGLCanvas2(uint32_t x, uint8_t y,
                                              uint32_t width, uint32_t height,
                                              void *process_loader) {
  GLInterface::InitGlobalInterface(process_loader);
  Matrix mvp = glm::ortho<float>(x, x + width, y + height, y);

  return std::make_unique<GLCanvas2>(mvp, width, height);
}

class GLCanvas2State final {
 public:
  GLCanvas2State() { this->InitStack(); }
  ~GLCanvas2State() = default;

  void Save() {
    Matrix current = matrix_stack_.back();

    matrix_stack_.emplace_back(current);
  }

  bool IsMatrixDirty() const { return matrix_dirty_; }

  void ClearMatrixDirty() { matrix_dirty_ = false; }

  bool HasClip() { return !clip_info_stack_.empty(); }

  Matrix CurrentMatrix() { return matrix_stack_.back(); }

  void UpdateCurrentMatrix(Matrix const &matrix) {
    matrix_stack_.back() = matrix;
    matrix_dirty_ = true;
  }

  void SaveClipRange(GLMeshRange const &range) {
    size_t depth = matrix_stack_.size();

    if (clip_info_stack_.empty() || clip_info_stack_.back() != depth) {
      clip_stack_.insert(std::make_pair(depth, range));
      clip_info_stack_.emplace_back(depth);
    } else {
      clip_stack_[depth] = range;
    }
  }

  void Restore(std::vector<std::unique_ptr<GLDrawOp2>> *ops) {
    size_t current_deepth = matrix_stack_.size();
    if (current_deepth == 1) {
      return;
    }

    Matrix prev = matrix_stack_.back();
    matrix_stack_.pop_back();

    this->HandleClipStack(current_deepth, ops);

    matrix_dirty_ = (prev != matrix_stack_.back()) || matrix_dirty_;
  }

 private:
  void InitStack() { matrix_stack_.emplace_back(glm::identity<Matrix>()); }
  void HandleClipStack(size_t prev_deepth,
                       std::vector<std::unique_ptr<GLDrawOp2>> *ops) {}

 private:
  std::vector<Matrix> matrix_stack_;
  std::unordered_map<size_t, GLMeshRange> clip_stack_;
  std::vector<size_t> clip_info_stack_;
  bool matrix_dirty_ = true;
};

GLCanvas2::~GLCanvas2() = default;

GLCanvas2::GLCanvas2(const Matrix &mvp, int32_t width, int32_t height)
    : Canvas(),
      mvp_(mvp),
      width_(width),
      height_(height),
      shader_(GLShader::CreateUniverseShader()),
      mesh_(std::make_unique<GLMesh>()),
      vertex_(std::make_unique<GLVertex2>()),
      texture_manager_(std::make_unique<GLTextureManager>()),
      state_(std::make_unique<GLCanvas2State>()) {
  // Init mesh
  mesh_->Init();
}

void GLCanvas2::onClipPath(const Path &path, Canvas::ClipOp op) {}

void GLCanvas2::onDrawPath(const Path &path, const Paint &paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  Rect bounds = path.getBounds();

  // step1 fill
  if (need_fill) {
    Path *temp = const_cast<Path *>(&path);
    if (paint.getPathEffect()) {
      paint.getPathEffect()->filterPath(temp, path, false, paint);
    }
    DoFillPath(temp, paint, bounds);
  }

  // step2 stroke
  if (need_stroke) {
    Path *temp = const_cast<Path *>(&path);
    if (paint.getPathEffect()) {
      paint.getPathEffect()->filterPath(temp, path, true, paint);
    }
    DoStrokePath(temp, paint, bounds);
  }
}

void GLCanvas2::onDrawGlyphs(const std::vector<GlyphInfo> &glyphs,
                             const Typeface *typeface, const Paint &paint) {
  Rect bounds{0, 0, 0, 0};
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;
  bool need_aa = paint.isAntiAlias();

  if (need_fill) {
    GLMeshRange fill_range{};

    fill_range.front_start = vertex_->FrontCount();
    fill_range.back_start = vertex_->BackCount();
    fill_range.aa_outline_start = vertex_->AACount();

    float advance_x = 0.f;
    for (const auto &glyph : glyphs) {
      float scale = paint.getTextSize() / glyph.font_size;

      Matrix sm = glm::scale(glm::identity<Matrix>(), {scale, scale, 1.f});
      Matrix tm =
          glm::translate(glm::identity<Matrix>(), {advance_x, 0.f, 0.f});

      Path temp = glyph.path.copyWithMatrix(tm * sm);

      GLFill2 gl_fill{paint, vertex_.get()};
      auto range = gl_fill.VisitPath(temp, true);

      if (need_aa) {
        Paint aa_paint{paint};

        aa_paint.setAntiAlias(true);
        aa_paint.setStrokeWidth(2.f);
        aa_paint.setStrokeMiter(2.4f);
        aa_paint.setStrokeCap(skity::Paint::kButt_Cap);
        aa_paint.setStrokeJoin(skity::Paint::kMiter_Join);
        GLStroke2 gl_stroke{aa_paint, vertex_.get()};
        gl_stroke.SetIsForAA(true);

        auto aa_range = gl_stroke.VisitPath(temp, true);

        range.aa_outline_start = aa_range.aa_outline_start;
        range.aa_outline_count = aa_range.aa_outline_count;
        range.quad_front_range = aa_range.quad_front_range;
      }

      fill_range.front_count += range.front_count;
      fill_range.back_count += range.back_count;
      fill_range.aa_outline_count += range.aa_outline_count;

      if (!range.quad_front_range.empty()) {
        fill_range.quad_front_range.insert(fill_range.quad_front_range.end(),
                                           range.quad_front_range.begin(),
                                           range.quad_front_range.end());
      }

      bounds.join(temp.getBounds());
      advance_x += glyph.advance_x * scale;
    }

    auto op = std::make_unique<GLDrawOpFill>(shader_.get(), mesh_.get(),
                                             fill_range, paint.isAntiAlias());

    if (paint.isAntiAlias()) {
      op->SetAAWidth(2.f);
    }

    SetupColorType(op.get(), paint, bounds, true);
    SetupUserTransform(op.get());

    gl_draw_ops_.emplace_back(std::move(op));
  }

  if (need_stroke) {
    GLMeshRange stroke_range{};

    stroke_range.front_start = vertex_->FrontCount();

    float advance_x = 0.f;

    for (const auto &glyph : glyphs) {
      float scale = paint.getTextSize() / glyph.font_size;

      Matrix sm = glm::scale(glm::identity<Matrix>(), {scale, scale, 1.f});
      Matrix tm =
          glm::translate(glm::identity<Matrix>(), {advance_x, 0.f, 0.f});

      Path temp = glyph.path.copyWithMatrix(tm * sm);

      GLStroke2 gl_stroke{paint, vertex_.get()};
      auto range = gl_stroke.VisitPath(temp, true);

      stroke_range.front_count += range.front_count;

      if (!range.quad_front_range.empty()) {
        stroke_range.quad_front_range.insert(
            stroke_range.quad_front_range.end(), range.quad_front_range.begin(),
            range.quad_front_range.end());
      }
      bounds.join(temp.getBounds());
      advance_x += glyph.advance_x * scale;
    }

    auto op = std::make_unique<GLDrawOpStroke>(
        shader_.get(), mesh_.get(), stroke_range, paint.getStrokeWidth(),
        paint.isAntiAlias());

    SetupColorType(op.get(), paint, bounds, false);
    SetupUserTransform(op.get());

    gl_draw_ops_.emplace_back(std::move(op));
  }
}

void GLCanvas2::onSave() { state_->Save(); }

void GLCanvas2::onRestore() { state_->Restore(&gl_draw_ops_); }

void GLCanvas2::onTranslate(float dx, float dy) {
  Matrix current = state_->CurrentMatrix();
  Matrix transform = glm::translate(glm::identity<Matrix>(), {dx, dy, 0.f});

  state_->UpdateCurrentMatrix(current * transform);
}

void GLCanvas2::onScale(float sx, float sy) {
  Matrix current = state_->CurrentMatrix();
  Matrix scale = glm::scale(glm::identity<Matrix>(), {sx, sy, 1.f});

  state_->UpdateCurrentMatrix(current * scale);
}

void GLCanvas2::onRotate(float degree) {
  Matrix current = state_->CurrentMatrix();
  Matrix rotate = glm::rotate(glm::identity<Matrix>(), glm::radians(degree),
                              {0.f, 0.f, 1.f});
  state_->UpdateCurrentMatrix(current * rotate);
}

void GLCanvas2::onRotate(float degree, float px, float py) {
  Matrix current = state_->CurrentMatrix();
  Matrix pre = glm::translate(glm::identity<Matrix>(), {-px, -py, 0.f});
  Matrix rotate = glm::rotate(glm::identity<Matrix>(), glm::radians(degree),
                              {0.f, 0.f, 1.f});
  Matrix post = glm::translate(glm::identity<Matrix>(), {px, py, 0.f});

  state_->UpdateCurrentMatrix(current * post * rotate * pre);
}

void GLCanvas2::onConcat(const Matrix &matrix) {
  Matrix current = state_->CurrentMatrix();

  state_->UpdateCurrentMatrix(current * matrix);
}

void GLCanvas2::onFlush() {
  mesh_->BindMesh();

  // setup OpenGL vertex buffer info
  this->UploadVertex();

  // FIXME: make sure VertexBuffer is bind
  mesh_->BindMesh();
  this->SetupGLVertexAttrib();

  shader_->Bind();

  shader_->SetMVPMatrix(mvp_);

  for (const auto &op : gl_draw_ops_) {
    op->Draw();
  }

  shader_->UnBind();
  mesh_->UnBindMesh();

  gl_draw_ops_.clear();
  vertex_->Reset();
}

uint32_t GLCanvas2::onGetWidth() const { return 0; }

uint32_t GLCanvas2::onGetHeight() const { return 0; }

void GLCanvas2::onUpdateViewport(uint32_t width, uint32_t height) {}

void GLCanvas2::UploadVertex() {
  auto vertex_data = vertex_->GetVertexDataSize();
  auto front_data = vertex_->GetFrontDataSize();
  auto back_data = vertex_->GetBackDataSize();
  auto aa_data = vertex_->GetAADataSize();
  auto quad_data = vertex_->GetQuadDataSize();

  if (std::get<1>(vertex_data) > 0) {
    mesh_->UploadVertexBuffer(std::get<0>(vertex_data),
                              std::get<1>(vertex_data));
  }

  if (std::get<1>(front_data) > 0) {
    mesh_->UploadFrontIndex(std::get<0>(front_data), std::get<1>(front_data));
  }

  if (std::get<1>(back_data) > 0) {
    mesh_->UploadBackIndex(std::get<0>(back_data), std::get<1>(back_data));
  }

  if (std::get<1>(aa_data) > 0) {
    mesh_->UploadAAOutlineIndex(std::get<0>(aa_data), std::get<1>(aa_data));
  }

  if (std::get<1>(quad_data) > 0) {
    mesh_->UploadQuadIndex(std::get<0>(quad_data), std::get<1>(quad_data));
  }
}

void GLCanvas2::SetupGLVertexAttrib() {
  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void *)0);
  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void *)(2 * sizeof(float)));
}

void GLCanvas2::DoFillPath(const Path *path, Paint const &paint,
                           Rect const &bounds) {
  GLFill2 gl_fill{paint, vertex_.get()};
  auto range = gl_fill.VisitPath(*path, true);

  if (paint.isAntiAlias()) {
    Paint aa_paint{paint};

    aa_paint.setAntiAlias(true);
    aa_paint.setStrokeWidth(2.f);
    aa_paint.setStrokeMiter(2.4f);
    aa_paint.setStrokeCap(skity::Paint::kButt_Cap);
    aa_paint.setStrokeJoin(skity::Paint::kMiter_Join);
    GLStroke2 gl_stroke{aa_paint, vertex_.get()};
    gl_stroke.SetIsForAA(true);

    auto aa_range = gl_stroke.VisitPath(*path, true);

    range.aa_outline_start = aa_range.aa_outline_start;
    range.aa_outline_count = aa_range.aa_outline_count;
    range.quad_front_range = aa_range.quad_front_range;
  }

  auto op = std::make_unique<GLDrawOpFill>(shader_.get(), mesh_.get(), range,
                                           paint.isAntiAlias());

  if (paint.isAntiAlias()) {
    op->SetAAWidth(2.f);
  }

  SetupColorType(op.get(), paint, bounds, true);
  SetupUserTransform(op.get());

  gl_draw_ops_.emplace_back(std::move(op));
}

void GLCanvas2::DoStrokePath(const Path *path, Paint const &paint,
                             Rect const &bounds) {
  GLStroke2 gl_stroke{paint, vertex_.get()};

  auto range = gl_stroke.VisitPath(*path, false);

  auto op = std::make_unique<GLDrawOpStroke>(shader_.get(), mesh_.get(), range,
                                             paint.getStrokeWidth(),
                                             paint.isAntiAlias());

  SetupColorType(op.get(), paint, bounds, false);
  SetupUserTransform(op.get());

  gl_draw_ops_.emplace_back(std::move(op));
}

void GLCanvas2::SetupColorType(GLDrawOp2 *op, Paint const &paint,
                               Rect const &bounds, bool fill) {
  if (paint.getShader()) {
    Shader::GradientInfo gradient_info{};
    Shader::GradientType type = paint.getShader()->asGradient(&gradient_info);
    auto pixmap = paint.getShader()->asImage();

    if (type != Shader::kNone) {
      // gradient color shader
      if (type == Shader::kLinear) {
        // linear gradient
        op->SetColorType(GLUniverseShader::kGradientLinear);
        // UserData4 in linear-gradient [p1.x, p1.y, p2.x, p2.y]
        op->SetUserData4({gradient_info.point[0].x, gradient_info.point[0].y,
                          gradient_info.point[1].x, gradient_info.point[1].y});
      } else if (type == Shader::kRadial) {
        // radial gradient
        op->SetColorType(GLUniverseShader::kGradientRadial);
        // UserData4 in radial-gradient [center.x, center.y, radialX, radialY]
        op->SetUserData4({gradient_info.point[0].x, gradient_info.point[0].y,
                          gradient_info.radius[0], gradient_info.radius[1]});
      }

      // UserData1 [color_type, alpha_flag, color_count, stop_count]
      op->SetUserData1({op->GetColorType(), gradient_info.gradientFlags,
                        gradient_info.colors.size(),
                        gradient_info.color_offsets.size()});
      op->SetUserShaderMatrix(gradient_info.local_matrix);

    } else if (pixmap) {
      // bitmap shader
      op->SetColorType(GLUniverseShader::kTexture);
      // UserData1 in bitmap shader
      op->SetUserData1({GLUniverseShader::kTexture, 0, 0, 0});
      // Setup GLTexture
      auto texture = texture_manager_->GenerateTexture(pixmap.get());
      op->SetGLTexture(texture);
      // UserData4 in bitmap shader [p1.x, p1.y, p2.x, p2.y];
      Vec2 p1{};
      Vec2 p2{};
      uint32_t pixmap_width = pixmap->Width();
      float width =
          std::min<float>(bounds.width(), static_cast<float>(pixmap->Width()));
      float height = std::min<float>(bounds.height(),
                                     static_cast<float>(pixmap->Height()));

      float x = bounds.left() + (bounds.width() - width) / 2.f;
      float y = bounds.top() + (bounds.height() - height) / 2.f;

      p1.x = x;
      p1.y = y;
      p2.x = x + width;
      p2.y = y + height;

      op->SetUserData4({p1, p2});
    }
    op->SetGradientColors(gradient_info.colors);
    op->SetGradientStops(gradient_info.color_offsets);

    // TODO support other shader color
    op->SetUserColor({1.f, 1.f, 1.f, paint.getAlphaF()});
  } else {
    op->SetColorType(GLUniverseShader::kPureColor);
    op->SetUserData1({GLUniverseShader::kPureColor, 0, 0, 0});
    Color4f color = fill ? paint.GetFillColor() : paint.GetStrokeColor();
    color.a *= paint.getAlphaF();
    op->SetUserColor(fill ? paint.GetFillColor() : paint.GetStrokeColor());
  }
}

void GLCanvas2::SetupUserTransform(GLDrawOp2 *op) {
  if (state_->IsMatrixDirty()) {
    op->SetUserTransform(state_->CurrentMatrix());
    state_->ClearMatrixDirty();
  }
}

}  // namespace skity
