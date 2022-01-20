#include <cmath>
#include <skity/effect/mask_filter.hpp>

namespace skity {

std::shared_ptr<MaskFilter> MaskFilter::MakeBlur(BlurStyle style,
                                                 float radius) {
  auto filter = std::make_shared<MaskFilter>();

  filter->style_ = style;
  filter->radius_ = radius;

  return filter;
}

Rect MaskFilter::approximateFilteredBounds(const Rect &src) const {
  float l = std::floor(src.left() - radius_);
  float t = std::floor(src.top() - radius_);
  float r = std::ceil(src.right() + radius_);
  float b = std::ceil(src.bottom() + radius_);

  return Rect::MakeLTRB(l, t, r, b);
}

}  // namespace skity