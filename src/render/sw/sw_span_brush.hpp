#ifndef SKITY_SRC_RENDER_SW_SW_SPAN_BRUSH_HPP
#define SKITY_SRC_RENDER_SW_SW_SPAN_BRUSH_HPP

#include "src/render/sw/sw_subpixel.hpp"

namespace skity {

class Bitmap;

class SWSpanBrush {
 public:
  SWSpanBrush(const Span* spans, size_t span_size, Bitmap* bitmap)
      : p_spans_(spans), spans_size_(span_size), bitmap_(bitmap) {}

  virtual ~SWSpanBrush() = default;

  virtual void Brush() = 0;

 protected:
  const Span* GetSpans() const { return p_spans_; }

  size_t GetSpanSize() const { return spans_size_; }

  Bitmap* CurrentBitmap() const { return bitmap_; }

 private:
  const Span* p_spans_;
  size_t spans_size_;
  Bitmap* bitmap_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_SW_SW_SPAN_BRUSH_HPP