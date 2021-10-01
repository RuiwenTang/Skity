
#include "src/render/gl/gl_canvas2.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "src/render/gl/gl_draw_op2.hpp"
#include "src/render/gl/gl_fill2.hpp"
#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_stroke2.hpp"
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

  void Save() {}

  Matrix CurrentMatrix() { return matrix_stack_.back(); }

  void UpdateCurrentMatrix(Matrix const &matrix) {
    matrix_stack_.back() = matrix;
  }

  void SaveClipRange(GLMeshRange const &range) {
    size_t depth = matrix_stack_.size();

    if (clip_stack_.count(depth) != 0) {
      clip_stack_[depth] = range;
    } else {
      clip_stack_.insert(std::make_pair(depth, range));
    }
  }

  void Restore(std::vector<std::unique_ptr<GLDrawOp2>> *ops) {
    size_t current_deepth = matrix_stack_.size();
    if (current_deepth == 1) {
      return;
    }

    matrix_stack_.pop_back();

    this->HandleClipStack(current_deepth, ops);
  }

 private:
  void InitStack() { matrix_stack_.emplace_back(glm::identity<Matrix>()); }
  void HandleClipStack(size_t prev_deepth,
                       std::vector<std::unique_ptr<GLDrawOp2>> *ops) {}

 private:
  std::vector<Matrix> matrix_stack_;
  std::unordered_map<size_t, GLMeshRange> clip_stack_;
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
      state_(std::make_unique<GLCanvas2State>()) {
  // Init mesh
  mesh_->Init();
}

void GLCanvas2::onClipPath(const Path &path, Canvas::ClipOp op) {}

void GLCanvas2::onDrawPath(const Path &path, const Paint &paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  // step1 fill
  if (need_fill) {
    DoFillPath(&path, paint);
  }

  // step2 stroke
  if (need_stroke) {
    DoStrokePath(&path, paint);
  }
}

void GLCanvas2::onDrawGlyphs(const std::vector<GlyphInfo> &glyphs,
                             const Typeface *typeface, const Paint &paint) {}
void GLCanvas2::onSave() {}
void GLCanvas2::onRestore() {}
void GLCanvas2::onTranslate(float dx, float dy) {}
void GLCanvas2::onScale(float sx, float sy) {}
void GLCanvas2::onRotate(float degree) {}
void GLCanvas2::onRotate(float degree, float px, float py) {}
void GLCanvas2::onConcat(const Matrix &matrix) {}

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

void GLCanvas2::DoFillPath(const Path *path, Paint const &paint) {
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

    auto aa_range = gl_stroke.VisitPath(*path, true);

    range.aa_outline_start = aa_range.front_start;
    range.aa_outline_count = aa_range.front_count;
    range.quad_front_range = aa_range.quad_front_range;
  }

  auto op = std::make_unique<GLDrawOpFill>(shader_.get(), mesh_.get(), range,
                                           paint.isAntiAlias());

  if (paint.isAntiAlias()) {
    op->SetAAWidth(2.f);
  }

  SetupColorType(op.get(), paint, true);

  gl_draw_ops_.emplace_back(std::move(op));
}

void GLCanvas2::DoStrokePath(const Path *path, Paint const &paint) {
  GLStroke2 gl_stroke{paint, vertex_.get()};

  auto range = gl_stroke.VisitPath(*path, false);

  auto op = std::make_unique<GLDrawOpStroke>(shader_.get(), mesh_.get(), range,
                                             paint.getStrokeWidth(),
                                             paint.isAntiAlias());

  SetupColorType(op.get(), paint, false);

  gl_draw_ops_.emplace_back(std::move(op));
}

void GLCanvas2::SetupColorType(GLDrawOp2 *op, Paint const &paint, bool fill) {
  // TODO support shader and texture
  op->SetColorType(GLUniverseShader::kPureColor);
  op->SetUserData1({GLUniverseShader::kPureColor, 0, 0, 0});
  op->SetUserColor(fill ? paint.GetFillColor() : paint.GetStrokeColor());
}

}  // namespace skity
