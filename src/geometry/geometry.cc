
#include "src/geometry/geometry.hpp"

namespace skity {

QuadCoeff::QuadCoeff(std::array<Point, 3> const& src) {
  C = FromPoint(src[0]);
  glm::vec2 P1 = FromPoint(src[1]);
  glm::vec2 P2 = FromPoint(src[2]);
  B = Times2(P1 - C);
  A = P2 - Times2(P1) + C;
}

Point QuadCoeff::evalAt(float t) { return Point{eval(t), 0, 1}; }

glm::vec2 QuadCoeff::eval(float t) { return eval(glm::vec2{t, t}); }

glm::vec2 QuadCoeff::eval(glm::vec2 const& tt) { return (A * tt + B) * tt + C; }

Point QuadCoeff::EvalQuadAt(std::array<Point, 3> const& src, float t) {
  return ToPoint(QuadCoeff{src}.eval(t));
}

void QuadCoeff::EvalQuadAt(std::array<Point, 3> const& src, float t,
                           Point* outP, Vector* outTangent) {
  if (t < 0) {
    t = 0;
  }
  if (t > Float1) {
    t = Float1;
  }

  if (outP) {
    *outP = EvalQuadAt(src, t);
  }

  if (outTangent) {
    *outTangent = EvalQuadTangentAt(src, t);
  }
}

Vector QuadCoeff::EvalQuadTangentAt(std::array<Point, 3> const& src, float t) {
  if ((t == 0 && src[0] == src[1]) || (t == 1 && src[1] == src[2])) {
    return src[2] - src[0];
  }

  glm::vec2 P0 = FromPoint(src[0]);
  glm::vec2 P1 = FromPoint(src[1]);
  glm::vec2 P2 = FromPoint(src[2]);

  glm::vec2 B = P1 - P0;
  glm::vec2 A = P2 - P1 - B;
  glm::vec2 T = A * glm::vec2{t, t} + B;

  return Vector{T + T, 0, 0};
}



}  // namespace skity