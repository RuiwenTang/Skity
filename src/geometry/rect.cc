#include <algorithm>
#include <skity/geometry/rect.hpp>

#include "src/geometry/math.hpp"

namespace skity {

float Rect::centerX() const { return FloatHalf * (right_ + left_); }

float Rect::centerY() const { return FloatHalf * (top_ + bottom_); }

Rect Rect::makeSorted() const {
  return MakeLTRB(std::min(left_, right_), std::min(top_, bottom_),
                  std::max(left_, right_), std::max(top_, bottom_));
}

bool Rect::isFinite() const {
  float accum = 0;
  accum *= left_;
  accum *= top_;
  accum *= right_;
  accum *= bottom_;

  return !FloatIsNan(accum);
}

}  // namespace skity