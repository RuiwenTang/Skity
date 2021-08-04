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
  Matrix matrix = glm::identity<Matrix>();
  // float angle = glm::acos(CrossProduct(start, stop) /
  //                         (glm::length(start) * glm::length(stop)));
  // matrix = glm::rotate(glm::identity<Matrix>(), angle, {0, 0, 1});
  if (dir == RotationDirection::kCCW) {
    matrix = glm::scale(matrix, {Float1, -Float1, 1});
  }

  if (userMatrix) {
    matrix = *userMatrix * matrix;
  }

  for (int i = 0; i < conicCount; i++) {
    for (int j = 0; j < 3; j++) {
      dst[i].pts[j] = matrix * dst[i].pts[j];
    }
  }

  return conicCount;
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

static void ratquad_mapTo3D(const Point src[3], float w, Vec3 dst[3]) {
  dst[0].x = src[0].x * 1.f;
  dst[0].y = src[0].y * 1.f;
  dst[0].z = 1.f;

  dst[1].x = src[1].x * w;
  dst[1].y = src[1].y * w;
  dst[1].z = w;

  dst[2].x = src[2].x * 1.f;
  dst[2].y = src[2].y * 1.f;
  dst[2].z = 1.f;
}

static inline Point project_down(Vec3 const& src) {
  return Point{src.x / src.z, src.y / src.z, 0.f, 1.f};
}

bool Conic::chopAt(float t, Conic dst[2]) const {
  std::array<Vec3, 3> tmp, tmp2;

  ratquad_mapTo3D(pts, w, tmp.data());

  P3DInterp(&tmp[0].x, &tmp2[0].x, t);
  P3DInterp(&tmp[0].y, &tmp2[0].y, t);
  P3DInterp(&tmp[0].z, &tmp2[0].z, t);

  dst[0].pts[0] = this->pts[0];
  dst[0].pts[1] = project_down(tmp2[0]);
  dst[0].pts[2] = project_down(tmp2[1]);

  dst[1].pts[0] = dst[0].pts[2];
  dst[1].pts[1] = project_down(tmp2[2]);
  dst[1].pts[2] = this->pts[2];

  // to put in "standard form", where w0 and w2 are both 1, we compute the
  // new w1 as sqrt(w1*w1/w0*w2)
  // or
  // w1 /= sqrt(w0*w2)
  //
  // However, in our case, we know that for dst[0]:
  //     w0 == 1, and for dst[1], w2 == 1
  //
  float root = std::sqrt(tmp2[1].z);
  dst[0].w = tmp2[0].z / root;
  dst[1].w = tmp2[2].z / root;
  return FloatIsFinite(dst[0].pts[0].x) && FloatIsFinite(dst[0].pts[0].y) &&
         FloatIsFinite(dst[0].pts[1].x) && FloatIsFinite(dst[0].pts[1].y) &&
         FloatIsFinite(dst[0].pts[2].x) && FloatIsFinite(dst[0].pts[2].y) &&
         FloatIsFinite(dst[0].w) && FloatIsFinite(dst[1].pts[0].x) &&
         FloatIsFinite(dst[1].pts[0].y) && FloatIsFinite(dst[1].pts[1].x) &&
         FloatIsFinite(dst[1].pts[1].y) && FloatIsFinite(dst[1].pts[2].x) &&
         FloatIsFinite(dst[1].pts[2].y) && FloatIsFinite(dst[1].w);
}

void Conic::chopAt(float t1, float t2, Conic* dst) const {
  if (0 == t1 || 1 == t2) {
    if (0 == t1 && 1 == t2) {
      *dst = *this;
      return;
    } else {
      Conic pair[2];
      if (this->chopAt(t1 ? t1 : t2, pair)) {
        *dst = pair[(t1 != 0) ? 1 : 0];
        return;
      }
    }
  }

  ConicCoeff coeff{*this};

  Vec2 tt1{t1};
  Vec2 aXY = coeff.numer.eval(tt1);
  Vec2 aZZ = coeff.denom.eval(tt1);
  Vec2 midTT{(t1 + t2) / 2};
  Vec2 dXY = coeff.numer.eval(midTT);
  Vec2 dZZ = coeff.denom.eval(midTT);

  Vec2 tt2{t2};
  Vec2 cXY = coeff.numer.eval(tt2);
  Vec2 cZZ = coeff.denom.eval(tt2);
  Vec2 bXY = Times2(dXY) - (aXY + cXY) * Vec2{0.5f};
  Vec2 bZZ = Times2(dZZ) - (aZZ + cZZ) * Vec2{0.5f};
  dst->pts[0] = ToPoint(aXY / aZZ);
  dst->pts[1] = ToPoint(bXY / bZZ);
  dst->pts[2] = ToPoint(cXY / cZZ);

  Vec2 ww = bZZ / (glm::sqrt(aZZ * cZZ));
  dst->w = ww[0];
}

uint32_t Conic::chopIntoQuadsPOW2(Point* pts, uint32_t pow2) {
  if (pow2 == kMaxConicToQuadPOW2) {
    std::array<Conic, 2> dst = {};
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

void Conic::evalAt(float t, Point* pt, Vector* tangent) const {
  assert(t >= 0 && t <= Float1);

  if (pt) {
    *pt = this->evalAt(t);
  }

  if (tangent) {
    *tangent = this->evalTangentAt(t);
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

  Vec2 p0 = FromPoint(pts[0]);
  Vec2 p1 = FromPoint(pts[1]);
  Vec2 p2 = FromPoint(pts[2]);

  Vec2 ww{this->w};

  Vec2 p20 = p2 - p0;
  Vec2 p10 = p1 - p0;

  Vec2 C = ww * p10;
  Vec2 A = ww * p20 - p20;
  Vec2 B = p20 - C - C;

  auto ret = QuadCoeff{A, B, C}.eval(t);

  return Vector{ret, 0, 0};
}

}  // namespace skity
