#ifndef SKITY_SRC_GEOMETRY_CONIC_HPP
#define SKITY_SRC_GEOMETRY_CONIC_HPP

#include <cstring>
#include <skity/geometry/point.hpp>

#include "src/geometry/geometry.hpp"

namespace skity {

struct Conic {
  enum { kMaxConicsForArc = 5 };

  static int BuildUnitArc(Vector const& start, Vector const& stop,
                          RotationDirection dir, Matrix* matrix,
                          Conic conics[kMaxConicsForArc]);

  Conic() = default;

  Conic(Point const& p0, Point const& p1, Point const& p2, float weight)
      : pts{p0, p1, p2}, w(weight)
  {
  }

  Conic(Point const p[3], float weight);

  void set(Point const p[3], float weight)
  {
    std::memcpy(pts, p, 3 * sizeof(Point));
    w = weight;
  }

  void set(const Point& p0, const Point& p1, const Point& p2, float weight)
  {
    pts[0] = p0;
    pts[1] = p1;
    pts[2] = p2;
    w = weight;
  }

  void evalAt(float t, Point* outP, Vector* outTangent = nullptr) const;
  Point evalAt(float t) const;
  Vector evalTangentAt(float t) const;

  Point pts[3];
  float w = 0.f;
};

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_CONIC_HPP
