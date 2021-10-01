
#include "src/render/gl/gl_canvas2.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "src/render/gl/gl_draw_op2.hpp"
#include "src/render/gl/gl_interface.hpp"
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
    clip_stack_.insert_or_assign(matrix_stack_.size(), range);
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
      state_(std::make_unique<GLCanvas2State>()) {}

void GLCanvas2::onClipPath(const Path &path, Canvas::ClipOp op) {}
void GLCanvas2::onDrawPath(const Path &path, const Paint &paint) {}
void GLCanvas2::onDrawGlyphs(const std::vector<GlyphInfo> &glyphs,
                             const Typeface *typeface, const Paint &paint) {}
void GLCanvas2::onSave() {}
void GLCanvas2::onRestore() {}
void GLCanvas2::onTranslate(float dx, float dy) {}
void GLCanvas2::onScale(float sx, float sy) {}
void GLCanvas2::onRotate(float degree) {}
void GLCanvas2::onRotate(float degree, float px, float py) {}
void GLCanvas2::onConcat(const Matrix &matrix) {}
void GLCanvas2::onFlush() {}
uint32_t GLCanvas2::onGetWidth() const { return 0; }
uint32_t GLCanvas2::onGetHeight() const { return 0; }
void GLCanvas2::onUpdateViewport(uint32_t width, uint32_t height) {}
}  // namespace skity
