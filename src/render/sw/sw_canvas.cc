#include "src/render/sw/sw_canvas.hpp"

#include <skity/graphic/bitmap.hpp>

#include "src/render/sw/sw_raster.hpp"
#include "src/render/sw/sw_span_brush.hpp"

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

void SWCanvas::onClipPath(const Path& path, ClipOp op) {}

void SWCanvas::onDrawPath(const Path& path, const Paint& paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  // Fill first
  if (need_fill) {
    SWRaster raster;

    raster.RastePath(path);

    auto brush = GenerateBrush(raster.CurrentSpans(), paint, false);

    brush->Brush();
  }
}

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

std::unique_ptr<SWSpanBrush> SWCanvas::GenerateBrush(
    std::vector<Span> const& spans, skity::Paint const& paint, bool stroke) {
  // TODO handle gradient and image

  Color4f color = stroke ? paint.getStrokeColor() : paint.getFillColor();

  return std::make_unique<SolidColorBrush>(spans, bitmap_, color);
}

}  // namespace skity