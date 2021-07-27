#include <cassert>
#include <skity/geometry/rrect.hpp>

#include "src/geometry/math.hpp"

namespace skity {

static bool are_radius_check_predicates_valid(float rad, float min, float max) {
  return (min <= max) && (rad <= max - min) && (min + rad <= max) &&
         (max - rad >= min) && rad >= 0;
}

RRect::Type RRect::getType() const {
  assert(this->isValid());
  return type_;
}

void RRect::setRect(Rect const& rect) {
  if (!this->initializeRect(rect)) {
    return;
  }

  this->radii_[0] = {0, 0};
  this->radii_[1] = {0, 0};
  this->radii_[2] = {0, 0};
  this->radii_[3] = {0, 0};

  this->type_ = Type::kRect;

  assert(this->isValid());
}

void RRect::setRectXY(Rect const& rect, float xRad, float yRad) {
  if (!this->initializeRect(rect)) {
    return;
  }

  if (!FloatIsFinite(xRad) && !FloatIsFinite(yRad)) {
    xRad = yRad = 0;
  }

  if (rect_.width() < xRad + xRad || rect_.height() < yRad + yRad) {
    float scale = std::min(SkityIEEEFloatDivided(rect_.width(), xRad + xRad),
                           SkityIEEEFloatDivided(rect_.height(), yRad + yRad));
    assert(scale < Float1);
    xRad *= scale;
    yRad *= scale;
  }

  if (xRad <= 0 || yRad <= 0) {
    this->setRect(rect);
    return;
  }

  for (int32_t i = 0; i < 4; i++) {
    radii_[i].x = xRad;
    radii_[i].y = yRad;
  }

  type_ = Type::kSimple;

  if (xRad >= FloatHalf * rect_.width() && yRad >= FloatHalf * rect_.height()) {
    type_ = Type::kOval;
  }

  assert(this->isValid());
}

void RRect::setOval(Rect const& oval) {
  if (!this->initializeRect(oval)) {
    return;
  }

  float x_rad = Rect::HalfWidth(rect_);
  float y_rad = Rect::HalfHeight(rect_);

  if (x_rad == 0.f || y_rad == 0.f) {
    type_ = kRect;
  } else {
    for (int32_t i = 0; i < 4; i++) {
      radii_[i].x = x_rad;
      radii_[i].y = y_rad;
    }
    type_ = kOval;
  }

  assert(this->isValid());
}

bool RRect::isValid() const {
  if (!AreRectAndRadiiValid(rect_, radii_)) {
    return false;
  }

  bool all_radii_zero = (0 == radii_[0].x && 0 == radii_[0].y);
  bool all_corners_square = (0 == radii_[0].x && 0 == radii_[0].y);
  bool all_radii_same = true;

  for (int32_t i = 1; i < 4; i++) {
    if (0 != radii_[i].x || 0 != radii_[i].y) {
      all_radii_zero = false;
    }

    if (radii_[i].x != radii_[i - 1].x || radii_[i].y != radii_[i - 1].y) {
      all_radii_same = false;
    }

    if (0 != radii_[i].x && 0 != radii_[i].y) {
      all_corners_square = false;
    }
  }

  // TODO support nine patch
  bool patches_of_nine = false;

  if (type_ < 0 || type_ > kLastType) {
    return false;
  }

  switch (type_) {
    case Type::kEmpty:
      if (!rect_.isEmpty() || !all_radii_zero || !all_radii_same ||
          !all_corners_square) {
        return false;
      }
      break;
    case Type::kRect:
      if (rect_.isEmpty() || !all_radii_zero || !all_radii_same ||
          !all_corners_square) {
        return false;
      }
      break;
    case Type::kOval:
      if (rect_.isEmpty() || all_radii_zero || !all_radii_same ||
          all_corners_square) {
        return false;
      }
      for (int32_t i = 0; i < 4; i++) {
        if (!FloatNearlyZero(radii_[i].x, Rect::HalfWidth(rect_)) ||
            !FloatNearlyZero(radii_[i].y, Rect::HalfHeight(rect_))) {
          return false;
        }
      }
      break;
    case Type::kSimple:
      if (rect_.isEmpty() || all_radii_zero || !all_radii_same ||
          all_corners_square) {
        return false;
      }
      break;
    case Type::kNinePatch:
      return false;
    case Type::kComplex:
      if (rect_.isEmpty() || all_radii_zero || all_radii_same ||
          all_corners_square) {
        return false;
      }
      break;
  }

  return true;
}

bool RRect::AreRectAndRadiiValid(Rect const& rect,
                                 std::array<Vec2, 4> const& radii) {
  if (!rect.isFinite() || !rect.isSorted()) {
    return false;
  }

  for (int32_t i = 0; i < 4; i++) {
    if (!are_radius_check_predicates_valid(radii[i].x, rect.left(),
                                           rect.right()) ||
        !are_radius_check_predicates_valid(radii[i].y, rect.top(),
                                           rect.bottom())) {
      return false;
    }
  }

  return true;
}

RRect RRect::MakeEmpty() { return RRect(); }

RRect RRect::MakeRect(Rect const& rect) {
  RRect rr;
  rr.setRect(rect);
  return rr;
}

RRect RRect::MakeRectXY(Rect const& rect, float xRad, float yRad) {
  RRect rr;
  rr.setRectXY(rect, xRad, yRad);
  return rr;
}

RRect RRect::MakeOval(Rect const& oval) {
  RRect rr;
  rr.setOval(oval);
  return rr;
}

bool RRect::initializeRect(Rect const& rect) {
  // Check this before sorting because sorting can hide nans.
  if (!rect.isFinite()) {
    *this = RRect();
    return false;
  }

  rect_ = rect.makeSorted();
  if (rect_.isEmpty()) {
    type_ = Type::kEmpty;
    return false;
  }

  return true;
}

}  // namespace skity