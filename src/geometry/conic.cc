#include "src/geometry/conic.hpp"

#include <array>
#include <cassert>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

#include "src/geometry/geometry.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

static float subdivide_w_value(float w) {
  return std::sqrt(FloatHalf + w * FloatHalf);
}

static bool between(float a, float b, float c) {
  return (a - b) * (c - b) <= 0;
}

static Point* subdivided(Conic const& src, Point* pts, uint32_t level) {
  if (0 == level) {
    std::memcpy(pts, &src.pts[1], 2 * sizeof(Point));
    return pts + 2;
  } else {
    std::array<Conic, 2> dst;
    src.chop(dst.data());
    float startY = src.pts[0].y;
    float endY = src.pts[2].y;
    if (between(startY, src.pts[1].y, endY)) {
      // If the input is monotonic and the output is not, the scan converter
      // hangs. Ensure that the chopped conics maintain their y-order.
      float midY = dst[0].pts[2].y;
      if (!between(startY, midY, endY)) {
        // If the computed midpoint is outside the ends,
        // move it to the closer one.
        float closerY =
            std::abs(midY - startY) < std::abs(midY - endY) ? startY : endY;
        dst[0].pts[2].y = dst[1].pts[0].y = closerY;
      }
      if (!between(startY, dst[0].pts[1].y, dst[0].pts[2].y)) {
        // If the 1st control is not between the start and end, put it at the
        // start. This also reduces the quad to a line.
        dst[0].pts[1].y = startY;
      }
      if (!between(dst[1].pts[0].y, dst[1].pts[1].y, endY)) {
        // If the 2nd control is not between the start and end, put it at the
        // end. This also reduces the quad to a line.
        dst[1].pts[1].y = endY;
      }
      // Verify that all five points are in order.
      assert(between(startY, dst[0].pts[1].y, dst[0].pts[2].y));
      assert(between(dst[0].pts[1].y, dst[0].pts[2].y, dst[1].pts[1].y));
      assert(between(dst[0].pts[2].y, dst[1].pts[1].y, endY));
    }
    --level;
    pts = subdivided(dst[0], pts, level);
    return subdivided(dst[1], pts, level);
  }
}

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

int Conic::BuildUnitArc(Vector const& start, Vector const& stop,
                        RotationDirection dir, Matrix* userMatrix,
                        Conic dst[kMaxConicsForArc]) {
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
  } else if (x == 0) {
    quadrant = y > 0 ? 1 : 3;
  } else {
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
  float angle = glm::acos(CrossProduct(start, stop) /
                          (glm::length(start) * glm::length(stop)));
  matrix = glm::rotate(glm::identity<Matrix>(), angle, {0, 0, 1});
  if (dir == RotationDirection::kCCW) {
    matrix = glm::scale(matrix, {Float1, -Float1, 1});
  }

  if (userMatrix) {
    matrix = matrix * *userMatrix;
  }

  for (int i = 0; i < conicCount; i++) {
    for (int j = 0; j < 3; j++) {
      dst[i].pts[j] = matrix * dst[i].pts[j];
    }
  }

  return conicCount;
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

void Conic::chop(Conic dst[2]) const {
  glm::vec2 scale{FloatInvert(this->w + Float1)};
  float new_weight = subdivide_w_value(this->w);
  glm::vec2 p0 = pts[0];
  glm::vec2 p1 = pts[1];
  glm::vec2 p2 = pts[2];
  glm::vec2 ww{this->w};

  glm::vec2 wp1 = ww * p1;
  glm::vec2 m = (p0 + Times2(wp1) + p2) * scale * glm::vec2(0.5f);

  Point mPt = ToPoint(m);
  if (!PointIsFinite(mPt)) {
    double w_d = this->w;
    double w_2 = w_d * 2;
    double scale_half = 1 / (1 + w_d) * 0.5;

    mPt.x =
        static_cast<float>((pts[0].x + w_2 * pts[1].x + pts[2].x) * scale_half);
    mPt.y =
        static_cast<float>((pts[0].y + w_2 * pts[1].y + pts[2].y) * scale_half);
  }

  dst[0].pts[0] = pts[0];
  dst[0].pts[1] = ToPoint((p0 + wp1) * scale);
  dst[0].pts[2] = dst[1].pts[0] = mPt;
  dst[1].pts[1] = ToPoint((wp1 + p2) * scale);
  dst[1].pts[2] = pts[2];

  dst[0].w = dst[1].w = new_weight;
}

uint32_t Conic::chopIntoQuadsPOW2(Point* pts, uint32_t pow2) {
  if (pow2 == kMaxConicToQuadPOW2) {
    std::array<Conic, 2> dst;
    this->chop(dst.data());
    // check to see if the first chop generates a pair of lines
    if (PointEqualsWithinTolerance(dst[0].pts[1], dst[0].pts[2]) &&
        PointEqualsWithinTolerance(dst[1].pts[1], dst[1].pts[2])) {
      // set ctrl == end to make lines
      pts[1] = pts[2] = pts[3] = dst[0].pts[1];
      pts[4] = dst[1].pts[2];
      pow2 = 1;
      goto commonFinitePtCheck;
    }
  }
  subdivided(*this, pts + 1, pow2);
commonFinitePtCheck:
  const int quadCount = 1 << pow2;
  const int ptCount = 2 * quadCount + 1;

  if (!PointAreFinite(pts, ptCount)) {
    for (int i = 1; i < ptCount - 1; ++i) {
      pts[i] = this->pts[1];
    }
  }

  return 1 << pow2;
}

}  // namespace skity
