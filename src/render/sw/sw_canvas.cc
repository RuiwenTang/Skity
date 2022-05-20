#include "src/render/sw/sw_canvas.hpp"

#include <skity/graphic/bitmap.hpp>

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeSoftwareCanvas(Bitmap* bitmap) {
  if (bitmap == nullptr) {
    return {};
  }

  if (bitmap->width() == 0 || bitmap->height() == 0) {
    return {};
  }

  return std::make_unique<SWCanvas>(bitmap);
}

SWCanvas::SWCanvas(Bitmap* bitmap) : Canvas(), bitmap_(bitmap) {}

void SWCanvas::onDrawLine(float x0, float y0, float x1, float y1,
                          Paint const& paint) {}

void SWCanvas::onDrawRect(Rect const& rect, Paint const& paint) {}

void SWCanvas::onClipPath(const Path& path, ClipOp op) {}

void SWCanvas::onDrawPath(const Path& path, const Paint& paint) {}

void SWCanvas::onDrawBlob(const TextBlob* blob, float x, float y,
                          Paint const& paint) {}

void SWCanvas::onSave() {}

void SWCanvas::onRestore() {}

void SWCanvas::onRestoreToCount(int saveCount) {}

void SWCanvas::onTranslate(float dx, float dy) {}

void SWCanvas::onScale(float sx, float sy) {}

void SWCanvas::onRotate(float degree) {}

void SWCanvas::onRotate(float degree, float px, float py) {}

void SWCanvas::onConcat(const Matrix& matrix) {}

void SWCanvas::onFlush() {}

uint32_t SWCanvas::onGetWidth() const { return bitmap_->width(); }

uint32_t SWCanvas::onGetHeight() const { return bitmap_->height(); }

void SWCanvas::onUpdateViewport(uint32_t width, uint32_t height) {}

}  // namespace skity