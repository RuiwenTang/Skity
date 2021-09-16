
#include "src/render/gl/gl_canvas2.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/render/gl/gl_interface.hpp"

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeGLCanvas2(uint32_t x, uint8_t y,
                                              uint32_t width, uint32_t height,
                                              void *process_loader) {
  GLInterface::InitGlobalInterface(process_loader);
  Matrix mvp = glm::ortho<float>(x, x + width, y + height, y);

  return std::make_unique<GLCanvas2>(mvp, width, height);
}

GLCanvas2::GLCanvas2(const Matrix &mvp, int32_t width, int32_t height)
    : Canvas(), mvp_(mvp), width_(width), height_(height) {}

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
