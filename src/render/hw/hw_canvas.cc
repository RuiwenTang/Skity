#include "src/render/hw/hw_canvas.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace skity {

HWCanvas::HWCanvas(Matrix mvp, uint32_t width, uint32_t height)
    : Canvas(), mvp_(mvp), width_(width), height_(height) {}

void HWCanvas::Init() { this->OnInit(); }

uint32_t HWCanvas::onGetWidth() const { return width_; }

uint32_t HWCanvas::onGetHeight() const { return height_; }

void HWCanvas::onUpdateViewport(uint32_t width, uint32_t height) {
  mvp_ = glm::ortho(0.f, 0.f, (float)height, (float)width);
  width_ = width;
  height_ = height;
}

void HWCanvas::onClipPath(const Path& path, ClipOp op) {}

void HWCanvas::onDrawPath(const Path& path, const Paint& paint) {}

void HWCanvas::onDrawGlyphs(const std::vector<GlyphInfo>& glyphs,
                            const Typeface* typeface, const Paint& paint) {}

void HWCanvas::onSave() { state_.Save(); }

void HWCanvas::onRestore() {
  // step 1 check if there is clip path need clean
  // step 2 restore state
  state_.Restore();
  // step 3 check if there is clip path need to apply
}

void HWCanvas::onTranslate(float dx, float dy) { state_.Translate(dx, dy); }

void HWCanvas::onScale(float sx, float sy) { state_.Scale(sx, sy); }

void HWCanvas::onRotate(float degree) { state_.Rotate(degree); }

void HWCanvas::onRotate(float degree, float px, float py) {
  state_.Rotate(degree, px, py);
}

void HWCanvas::onConcat(const Matrix& matrix) { state_.Concat(matrix); }

void HWCanvas::onFlush() {}

}  // namespace skity