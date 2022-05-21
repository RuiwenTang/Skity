#ifndef SKITY_SRC_RENDER_SW_SW_SPAN_BRUSH_HPP
#define SKITY_SRC_RENDER_SW_SW_SPAN_BRUSH_HPP

#include <skity/graphic/color.hpp>
#include <vector>

#include "src/render/sw/sw_subpixel.hpp"

namespace skity {

class Bitmap;

class SWSpanBrush {
 public:
  SWSpanBrush(std::vector<Span> const& spans, Bitmap* bitmap)
      : p_spans_(spans.data()), spans_size_(spans.size()), bitmap_(bitmap) {}

  virtual ~SWSpanBrush() = default;

  void Brush();

 protected:
  virtual Color4f CalculateColor(int32_t x, int32_t y) = 0;

  const Span* GetSpans() const { return p_spans_; }

  size_t GetSpanSize() const { return spans_size_; }

  Bitmap* CurrentBitmap() const { return bitmap_; }

 private:
  const Span* p_spans_;
  size_t spans_size_;
  Bitmap* bitmap_;
};

class SolidColorBrush : public SWSpanBrush {
 public:
  SolidColorBrush(std::vector<Span> const& spans, Bitmap* bitmap, Color4f color)
      : SWSpanBrush(spans, bitmap), color_(std::move(color)) {}

  ~SolidColorBrush() override = default;

 protected:
  Color4f CalculateColor(int32_t x, int32_t y) override;

 private:
  Color4f color_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_SW_SW_SPAN_BRUSH_HPP