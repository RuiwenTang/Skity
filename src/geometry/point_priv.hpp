#ifndef SKITY_SRC_GEOMETRY_POINT_PRIV_HPP
#define SKITY_SRC_GEOMETRY_POINT_PRIV_HPP

#include <cmath>
#include <skity/geometry/point.hpp>

#include "src/geometry/geometry.hpp"

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

static inline void PointScale(Point const& src, float scale, Point* dst)
{
  dst->x = src.x * scale;
  dst->y = src.y * scale;
  dst->z = 0;
  dst->w = 1;
}

static inline bool PointEqualsWithinTolerance(Point const& pt, Point const& p,
                                              float tol)
{
  return FloatNearlyZero(pt.x - p.x, tol) && FloatNearlyZero(pt.y - p.y, tol);
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

static inline bool VectorSetNormal(Vector& vec, float x, float y)
{
  return PointSetLength<false>(vec, x, y, Float1);
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

static inline void PointRotateCW(Point const& src, Point* dst)
{
  float tmp = src.x;
  dst->x = -src.y;
  dst->y = tmp;
}

static inline void PointRotateCW(Point* pt) { PointRotateCW(*pt, pt); }

static inline void PointRotateCCW(Point const& src, Point* dst)
{
  float tmp = src.x;
  dst->x = src.y;
  dst->y = -tmp;
}

static inline void PointRotateCCW(Point* pt) { PointRotateCCW(*pt, pt); }

static inline bool PointsWithInDist(Point const& nearPt, Point const& farPt,
                                    float limit)
{
  return PointDistanceToSqd(nearPt, farPt) <= limit * limit;
}

static inline bool SharpAngle(const Point quad[3])
{
  Vector smaller = quad[1] - quad[0];
  Vector larger = quad[1] - quad[2];

  float smallLen = PointLengthSqd(smaller);
  float largerLen = PointLengthSqd(larger);

  if (smallLen > largerLen) {
    std::swap(smaller, larger);
    largerLen = smallLen;
  }

  if (!PointSetLength<false>(smaller, smaller.x, smaller.y, largerLen)) {
    return false;
  }

  float dot = glm::dot(glm::vec2{smaller}, glm::vec2{larger});
  return dot > 0;
}

static inline int32_t IntersectQuadRay(const Point line[2], const Point quad[3],
                                       float roots[2])
{
  Vector vec = line[1] - line[0];
  float r[3];
  for (int32_t n = 0; n < 3; n++) {
    r[n] = (quad[n].y - line[0].y) * vec.x - (quad[n].x - line[0].x) * vec.y;
  }

  float A = r[2];
  float B = r[1];
  float C = r[0];
  A += C - 2 * B;
  B -= C;
  return FindUnitQuadRoots(A, 2 * B, C, roots);
}

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_POINT_PRIV_HPP
