#include <skity/effect/mask_filter.hpp>

namespace skity {

std::shared_ptr<MaskFilter> MaskFilter::MakeBlur(BlurStyle style, float sigma) {
  return std::make_shared<MaskFilter>();
}

Rect MaskFilter::approximateFilteredBounds(const Rect &src) const {
  return Rect();
}

}  // namespace skity