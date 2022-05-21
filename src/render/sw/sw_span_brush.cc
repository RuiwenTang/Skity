#include "src/render/sw/sw_span_brush.hpp"

#include <skity/graphic/bitmap.hpp>

namespace skity {

void SWSpanBrush::Brush() {
  for (size_t i = 0; i < spans_size_; i++) {
    Span const& span = p_spans_[i];
    Color4f color = CalculateColor(span.x, span.y);

    color.a *= (span.cover / 255.f);
    for (size_t l = 0; l < span.len; l++) {
      bitmap_->blendPixel(span.x + l, span.y, color);
    }
  }
}

Color4f SolidColorBrush::CalculateColor(int32_t x, int32_t y) { return color_; }

}  // namespace skity