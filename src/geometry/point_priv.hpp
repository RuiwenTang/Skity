#ifndef SKITY_SRC_GEOMETRY_POINT_PRIV_HPP
#define SKITY_SRC_GEOMETRY_POINT_PRIV_HPP

#include <cmath>
#include <skity/geometry/point.hpp>

#include "src/geometry/geometry.hpp"

namespace skity {

static inline bool PointIsFinite(Point const& point) {
  return std::isfinite(point.x) && std::isfinite(point.y) &&
         std::isfinite(point.z) && std::isfinite(point.w);
}

static inline bool PointAreFinite(Point points[], uint32_t count) {
  float prod = 0;

  for (uint32_t i = 0; i < count; i++) {
    prod *= (points[i].x * points[i].y);
  }

  return prod == 0;
}

static inline void PointSet(Point& point, float x, float y) {
  point.x = x;
  point.y = y;
  point.z = 0;
  point.w = 1;
}

static inline void VectorSet(Vector& vec, float x, float y) {
  vec.x = x;
  vec.y = y;
  vec.z = 0;
  vec.w = 0;
}

static inline void PointScale(Point const& src, float scale, Point* dst) {
  dst->x = src.x * scale;
  dst->y = src.y * scale;
  dst->z = 0;
  dst->w = 1;
}

static inline void PointRotateCCW(Point const& src, Point* dst) {
  float tmp = src.x;
  dst->x = src.y;
  dst->y = -tmp;
  dst->z = 0.f;
  dst->w = 1.f;
}

static inline void PointRotateCCW(Point* pt) { PointRotateCCW(*pt, pt); }

static inline void PointRotateCW(Point const& src, Point* dst) {
  float tmp = src.x;
  dst->x = -src.y;
  dst->y = tmp;
  dst->z = 0.f;
  dst->w = 1.f;
}

static inline void PointRotateCW(Point* pt) { PointRotateCW(*pt, pt); }

static inline bool PointEqualsWithinTolerance(Point const& pt, Point const& p,
                                              float tol) {
  return FloatNearlyZero(pt.x - p.x, tol) && FloatNearlyZero(pt.y - p.y, tol);
}

template <bool use_rsqrt, class POINT>
bool PointSetLength(POINT& pt, float x, float y, float length,
                    float* orig_length = nullptr) {
  double xx = x;
  double yy = y;
  double dmag = glm::sqrt(xx * xx + yy * yy);
  double dscale = length / dmag;

  x *= dscale;
  y *= dscale;

  if (glm::isinf(x) || glm::isinf(y) || (x == 0 && y == 0)) {
    pt.x = 0;
    pt.y = 0;
    return false;
  }

  float mag = 0;
  if (orig_length) {
    mag = static_cast<float>(dmag);
  }
  pt.x = x;
  pt.y = y;
  if (orig_length) {
    *orig_length = mag;
  }

  return true;
}

static inline bool VectorSetNormal(Vector& vec, float x, float y) {
  return PointSetLength<false>(vec, x, y, Float1);
}

static inline float PointDistanceToSqd(Point const& pt, Point const& a) {
  float dx = pt.x - a.x;
  float dy = pt.y - a.y;
  return dx * dx + dy * dy;
}

static inline float PointDistance(Point const& a, Point const& b) {
  return glm::length(glm::vec2(a - b));
}

static inline float PointLengthSqd(Point const& pt) {
  return glm::dot(glm::vec2{pt}, glm::vec2{pt});
}

static inline bool PointCanNormalize(float dx, float dy) {
  return (!glm::isinf(dx) && !glm::isinf(dy)) && (dx || dy);
}

template <class POINT>
static inline bool PointEqualsWithinTolerance(POINT const& p1,
                                              POINT const& p2) {
  return !PointCanNormalize(p1.x - p2.x, p1.y - p2.y);
}

template <glm::length_t L, typename T, glm::qualifier Q>
T VectorDotProduct(glm::vec<L, T, Q> const& a, glm::vec<L, T, Q> const& b) {
  return a.x * b.y - a.y * b.x;
}

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_POINT_PRIV_HPP
