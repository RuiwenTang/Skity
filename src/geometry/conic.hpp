#ifndef SKITY_SRC_GEOMETRY_CONIC_HPP
#define SKITY_SRC_GEOMETRY_CONIC_HPP

#include <array>
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
      : pts{p0, p1, p2}, w(weight) {}

  Conic(Point const p[3], float weight);

  

  std::array<Point, 3> pts = {};
  float w = 0.f;
};

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_CONIC_HPP