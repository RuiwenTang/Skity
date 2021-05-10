#include "src/geometry/conic.hpp"

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

#include "src/geometry/geometry.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

Conic::Conic(Point const p[3], float weight) : pts{p[0], p[1], p[2]}, w(weight)
{
}

void Conic::evalAt(float t, Point* outP, Vector* outTangent) const
{
  assert(t >= 0 && t <= Float1);

  if (outP) {
    *outP = evalAt(t);
  }

  if (outTangent) {
    *outTangent = evalTangentAt(t);
  }
}

int Conic::BuildUnitArc(Vector const& start, Vector const& stop,
                        RotationDirection dir, Matrix* userMatrix,
                        Conic dst[kMaxConicsForArc])
{
  // rotate by x,y so that uStart is (1.0)
//  float x = glm::dot(start, stop);
  float x = start.x * stop.x + start.y * stop.y;
  float y = CrossProduct(start, stop);

  float absY = glm::abs(y);
  if (absY <= NearlyZero && x > 0 &&
      ((y >= 0 && dir == RotationDirection::kCW) ||
       (y <= 0 && dir == RotationDirection::kCCW))) {
    return 0;
  }

  if (dir == RotationDirection::kCCW) {
    y = -y;
  }

  // We decide to use 1-conic per quadrant of a circle. What quadrant does [xy]
  // lie in?
  //      0 == [0  .. 90)
  //      1 == [90 ..180)
  //      2 == [180..270)
  //      3 == [270..360)
  //
  int quadrant = 0;
  if (y == 0) {
    quadrant = 2;
  }
  else if (x == 0) {
    quadrant = y > 0 ? 1 : 3;
  }
  else {
    if (y < 0) {
      quadrant += 2;
    }

    if ((x < 0) != (y < 0)) {
      quadrant += 1;
    }
  }

  const Point quadrantPts[] = {
      {1, 0, 0, 1},  {1, 1, 0, 1},   {0, 1, 0, 1},  {-1, 1, 0, 1},
      {-1, 0, 0, 1}, {-1, -1, 0, 1}, {0, -1, 0, 1}, {1, -1, 0, 1},
  };

  const float quadrantWeight = FloatRoot2Over2;

  int conicCount = quadrant;
  for (int i = 0; i < conicCount; i++) {
    dst[i].set(std::addressof(quadrantPts[i * 2]), quadrantWeight);
  }

  // Now compute any remaing (sub-90-degree) arc for the last conic
  const Point finalP = {x, y, 0, 1};
  const Point& lastQ = quadrantPts[quadrant * 2];
  const float dot = glm::dot(lastQ, finalP);

  if (dot < 1) {
    Vector offCurve = {lastQ.x + x, lastQ.y + y, 0, 0};
    const float cosThetaOver2 = glm::sqrt((1 + dot) / 2);
    PointSetLength<false>(offCurve, offCurve.x, offCurve.y,
                          FloatInvert(cosThetaOver2));
    offCurve.z = 0;
    offCurve.w = 0;
    if (!PointEqualsWithinTolerance(lastQ, offCurve)) {
      dst[conicCount].set(lastQ, offCurve, finalP, cosThetaOver2);
      conicCount += 1;
    }
  }

  // now handle counter-clockwise and the initial unitStart rotation
  Matrix matrix;
  float angle = glm::acos(start.x);
  matrix = glm::rotate(glm::identity<Matrix>(), angle, {0, 0, 1});
  if (dir == RotationDirection::kCCW) {
    matrix = glm::scale(glm::identity<Matrix>(), {Float1, -Float1, 1}) * matrix;
  }

  if (userMatrix) {
    matrix = matrix * *userMatrix;
  }

  for (int i = 0; i < conicCount; i++) {
    for (int j = 0; j < 3; j++) {
      dst[i].pts[j] = dst[i].pts[j] * matrix;
    }
  }

  return conicCount;
}

Point Conic::evalAt(float t) const
{
  return ToPoint(ConicCoeff{*this}.eval(t));
}

Vector Conic::evalTangentAt(float t) const
{
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
