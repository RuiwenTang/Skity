#include "src/geometry/conic.hpp"

#include <cassert>

#include "src/geometry/geometry.hpp"

namespace skity {

Conic::Conic(Point const p[3], float weight)
    : pts{p[0], p[1], p[2]}, w(weight) {}

void Conic::evalAt(float t, Point* outP, Vector* outTangent) const {
  assert(t >= 0 && t <= Float1);

  if (outP) {
    *outP = evalAt(t);
  }

  if (outTangent) {
    *outTangent = evalTangentAt(t);
  }
}

Point Conic::evalAt(float t) const {
  return ToPoint(ConicCoeff{*this}.eval(t));
}

Vector Conic::evalTangentAt(float t) const {
  // The derivative equation returns a zero tangent vector when t is 0 or 1,
  // and the control point is equal to the end point.
  // In this case, use the conic endpoints to compute the tangent.
  if ((t == 0 && pts[0] == pts[1]) || (t == 1 && pts[1] == pts[2])) {
    return pts[2] - pts[0];
  }

  glm::vec2 p0 = FromPoint(pts[0]);
  glm::vec2 p1 = FromPoint(pts[1]);
  glm::vec2 p2 = FromPoint(pts[2]);
  glm::vec2 ww{w, w};

  glm::vec2 p20 = p2 - p0;
  glm::vec2 p10 = p1 - p0;

  glm::vec2 C = ww * p10;
  glm::vec2 A = ww * p20 - p20;
  glm::vec2 B = p20 - C - C;

  return Vector{QuadCoeff(A, B, C).eval(t), 0, 0};
}

}  // namespace skity