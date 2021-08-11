#include <algorithm>
#include <skity/geometry/rect.hpp>

#include "src/geometry/math.hpp"

namespace skity {

bool Rect::setBoundsCheck(const Point* pts, int count) {
  if (count <= 0) {
    setEmpty();
    return true;
  }

  glm::vec2 min, max;
  if (count & 1) {
    min = max = {pts->x, pts->y};
    pts += 1;
    count -= 1;
  } else {
    min = glm::min(pts[0], pts[1]);
    max = glm::min(pts[0], pts[1]);
    pts += 2;
    count -= 2;
  }

  glm::vec2 accum = min * float(0);
  while (count) {
    glm::vec2 x = pts[0];
    glm::vec2 y = pts[1];
    accum *= x;
    accum *= y;
    min = glm::min(min, glm::min(x, y));
    max = glm::max(max, glm::max(x, y));
    pts += 2;
    count -= 2;
  }

  accum *= 0.f;
  bool all_finite = !glm::isinf(accum.x) && !glm::isinf(accum.y);
  if (all_finite) {
    this->setLTRB(min.x, min.y, max.x, max.y);
  } else {
    this->setEmpty();
  }
  return all_finite;
}

float Rect::centerX() const { return FloatHalf * (right_ + left_); }

float Rect::centerY() const { return FloatHalf * (top_ + bottom_); }

Rect Rect::makeSorted() const {
  return MakeLTRB(std::min(left_, right_), std::min(top_, bottom_),
                  std::max(left_, right_), std::max(top_, bottom_));
}

void Rect::join(const Rect& r) {
  if (r.isEmpty()) {
    return;
  }

  if (this->isEmpty()) {
    *this = r;
  } else {
    left_ = std::min(left_, r.left_);
    top_ = std::min(top_, r.top_);
    right_ = std::max(right_, r.right_);
    bottom_ = std::max(bottom_, r.bottom_);
  }
}

bool Rect::isFinite() const {
  float accum = 0;
  accum *= left_;
  accum *= top_;
  accum *= right_;
  accum *= bottom_;

  return !FloatIsNan(accum);
}

float Rect::HalfWidth(Rect const& rect) {
  return rect.right_ * FloatHalf - rect.left_ * FloatHalf;
}

float Rect::HalfHeight(Rect const& rect) {
  return rect.bottom_ * FloatHalf - rect.top_ * FloatHalf;
}

}  // namespace skity