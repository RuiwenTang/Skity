#ifndef SKITY_SRC_GEOMETRY_POINT_PRIV_HPP
#define SKITY_SRC_GEOMETRY_POINT_PRIV_HPP

#include <cmath>
#include <skity/geometry/point.hpp>

namespace skity {

static inline bool PointIsFinite(Point const& point)
{
  return std::isfinite(point.x) && std::isfinite(point.y) &&
         std::isfinite(point.z) && std::isfinite(point.w);
}

static inline void PointSet(Point& point, float x, float y)
{
  point.x = x;
  point.y = y;
  point.z = 0;
  point.w = 1;
}

static inline void VectorSet(Vector& vec, float x, float y)
{
  vec.x = x;
  vec.y = y;
  vec.z = 0;
  vec.w = 0;
}

template <bool use_rsqrt>
bool PointSetLength(Point& pt, float x, float y, float length,
                    float* orig_length = nullptr)
{
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

static inline float PointDistanceToSqd(Point const& pt, Point const& a)
{
  float dx = pt.x - a.x;
  float dy = pt.y - a.y;
  return dx * dx + dy * dy;
}

static inline float PointLengthSqd(Point const& pt)
{
  return glm::dot(glm::vec2{pt}, glm::vec2{pt});
}

static inline bool PointCanNormalize(float dx, float dy)
{
  return (!glm::isinf(dx) && !glm::isinf(dy)) && (dx || dy);
}

template <glm::length_t L, typename T, glm::qualifier Q>
T VectorDotProduct(glm::vec<L, T, Q> const& a, glm::vec<L, T, Q> const& b)
{
  return a.x * b.y - a.y * b.x;
}

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_POINT_PRIV_HPP

