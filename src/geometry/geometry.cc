
#include "src/geometry/geometry.hpp"

#include <cassert>

#include "src/geometry/conic.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

static Vector eval_cubic_derivative(const Point src[4], float t) {
  QuadCoeff coeff;
  glm::vec2 P0 = FromPoint(src[0]);
  glm::vec2 P1 = FromPoint(src[1]);
  glm::vec2 P2 = FromPoint(src[2]);
  glm::vec2 P3 = FromPoint(src[3]);

  coeff.A = P3 + glm::vec2{3, 3} * (P1 - P2) - P0;
  coeff.B = Times2(P2 - Times2(P1) + P0);
  coeff.C = P1 - P0;
  glm::vec2 ret = coeff.eval(t);
  return Vector{ret.x, ret.y, 0, 0};
}

static Vector eval_cubic_2ndDerivative(const Point src[4], float t) {
  glm::vec2 P0 = FromPoint(src[0]);
  glm::vec2 P1 = FromPoint(src[1]);
  glm::vec2 P2 = FromPoint(src[2]);
  glm::vec2 P3 = FromPoint(src[3]);
  glm::vec2 A = P3 + glm::vec2{3, 3} * (P1 - P2) - P0;
  glm::vec2 B = P2 - Times2(P1) + P0;

  glm::vec2 vec = A * glm::vec2{t, t} + B;
  return glm::vec4{vec.x, vec.y, 0, 0};
}

static float calc_cubic_precision(const Point src[4]) { return 0; }

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

void QuadCoeff::ChopQuadAt(const Point src[3], Point dst[5], float t) {
  assert(t > 0 && t < Float1);

  Vec2 p0 = FromPoint(src[0]);
  Vec2 p1 = FromPoint(src[1]);
  Vec2 p2 = FromPoint(src[2]);
  Vec2 tt{t};

  Vec2 p01 = Interp(p0, p1, tt);
  Vec2 p12 = Interp(p1, p2, tt);

  dst[0] = ToPoint(p0);
  dst[1] = ToPoint(p01);
  dst[2] = ToPoint(Interp(p01, p12, tt));
  dst[3] = ToPoint(p12);
  dst[4] = ToPoint(p2);
}

CubicCoeff::CubicCoeff(std::array<Point, 4> const& src) {
  glm::vec2 P0 = FromPoint(src[0]);
  glm::vec2 P1 = FromPoint(src[1]);
  glm::vec2 P2 = FromPoint(src[2]);
  glm::vec2 P3 = FromPoint(src[3]);
  glm::vec2 three{3, 3};

  A = P3 + three * (P1 - P2) - P0;
  B = three * (P2 - Times2(P1) + P0);
  C = three * (P1 - P0);
  D = P0;
}

Point CubicCoeff::evalAt(float t) { return Point{eval(t), 0, 1}; }

glm::vec2 CubicCoeff::eval(float t) { return eval(glm::vec2{t, t}); }

glm::vec2 CubicCoeff::eval(glm::vec2 const& t) {
  return ((A * t + B) * t + C) * t + D;
}

void CubicCoeff::EvalCubicAt(const Point src[4], float t, Point* loc,
                             Vector* tangent, Vector* curvature) {
  if (loc) {
    *loc = ToPoint(CubicCoeff({src[0], src[1], src[2], src[3]}).eval(t));
  }

  if (tangent) {
    // The derivative equation returns a zero tangent vector when t is 0 or 1,
    // and the adjacent control point is equal to the end point. In this case,
    // use the next control point or the end points to compute the tangent.
    if ((t == 0 && src[0] == src[1]) || (t == 1 && src[2] == src[3])) {
      if (t == 0) {
        *tangent = src[2] - src[0];
      } else {
        *tangent = src[3] - src[1];
      }

      if (!tangent->x && !tangent->y) {
        *tangent = src[3] - src[0];
      }
    } else {
      *tangent = eval_cubic_derivative(src, t);
    }
  }

  if (curvature) {
    *curvature = eval_cubic_2ndDerivative(src, t);
  }
}

void CubicCoeff::ChopCubicAt(const Point src[4], Point dst[7], float t) {
  glm::vec2 p0 = FromPoint(src[0]);
  glm::vec2 p1 = FromPoint(src[1]);
  glm::vec2 p2 = FromPoint(src[2]);
  glm::vec2 p3 = FromPoint(src[3]);
  glm::vec2 tt{t, t};

  glm::vec2 ab = Interp(p0, p1, tt);
  glm::vec2 bc = Interp(p1, p2, tt);
  glm::vec2 cd = Interp(p2, p3, tt);
  glm::vec2 abc = Interp(ab, bc, tt);
  glm::vec2 bcd = Interp(bc, cd, tt);
  glm::vec2 abcd = Interp(abc, bcd, tt);

  dst[0] = ToPoint(p0);
  dst[1] = ToPoint(ab);
  dst[2] = ToPoint(abc);
  dst[3] = ToPoint(abcd);
  dst[4] = ToPoint(bcd);
  dst[5] = ToPoint(cd);
  dst[6] = ToPoint(p3);
}

ConicCoeff::ConicCoeff(Conic const& conic) {
  glm::vec2 P0 = FromPoint(conic.pts[0]);
  glm::vec2 P1 = FromPoint(conic.pts[1]);
  glm::vec2 P2 = FromPoint(conic.pts[2]);
  glm::vec2 ww{conic.w, conic.w};

  glm::vec2 p1w = P1 * ww;
  numer.C = P0;
  numer.A = P2 - Times2(p1w) + P0;
  numer.B = Times2(p1w - P0);

  denom.C = glm::vec2{1, 1};
  denom.B = Times2(ww - denom.C);
  denom.A = glm::vec2{0, 0} - denom.B;
}

glm::vec2 ConicCoeff::eval(float t) {
  glm::vec2 tt{t, t};
  glm::vec2 n = numer.eval(tt);
  glm::vec2 d = denom.eval(tt);
  return n / d;
}

bool DegenerateVector(Vector const& v) { return !PointCanNormalize(v.x, v.y); }

float pt_to_line(Point const& pt, Point const& lineStart,
                 Point const& lineEnd) {
  Vector dxy = lineEnd - lineStart;
  Vector ab0 = pt - lineStart;

  float number = VectorDotProduct(dxy, ab0);
  float denom = VectorDotProduct(dxy, dxy);
  float t = SkityIEEEFloatDivided(number, denom);
  if (t >= 0 && t <= 1) {
    Point hit;
    hit.x = lineStart.x * (1 - t) + lineEnd.x * t;
    hit.y = lineStart.y * (1 - t) + lineEnd.y * t;
    hit.z = 0;
    hit.w = 1;
    return PointDistanceToSqd(hit, pt);
  } else {
    return PointDistanceToSqd(pt, lineStart);
  }
}

void SubDividedCubic(const Point cubic[4], Point sub_cubic1[4],
                     Point sub_cubic2[4]) {
  Point p1 = (cubic[0] + cubic[1]) / 2.f;
  Point p2 = (cubic[1] + cubic[2]) / 2.f;
  Point p3 = (cubic[2] + cubic[3]) / 2.f;
  Point p4 = (p1 + p2) / 2.f;
  Point p5 = (p2 + p3) / 2.f;
  Point p6 = (p4 + p5) / 2.f;

  Point p0 = cubic[0];
  Point p7 = cubic[3];

  sub_cubic1[0] = p0;
  sub_cubic1[1] = p1;
  sub_cubic1[2] = p4;
  sub_cubic1[3] = p6;

  sub_cubic2[0] = p6;
  sub_cubic2[1] = p5;
  sub_cubic2[2] = p3;
  sub_cubic2[3] = p7;
}

void SubDividedCubic2(const Point cubic[4], Point sub_cubic[8]) {
  SubDividedCubic(cubic, sub_cubic, sub_cubic + 4);
}

void SubDividedCubic4(const Point cubic[4], Point sub_cubic[16]) {
  SubDividedCubic(cubic, sub_cubic, sub_cubic + 8);
  SubDividedCubic2(sub_cubic, sub_cubic);
  SubDividedCubic2(sub_cubic + 8, sub_cubic + 8);
}

void SubDividedCubic8(const Point cubic[4], Point sub_cubic[32]) {
  SubDividedCubic(cubic, sub_cubic, sub_cubic + 16);
  SubDividedCubic4(sub_cubic, sub_cubic);
  SubDividedCubic4(sub_cubic + 16, sub_cubic + 16);
}

void SubDividedQuad(const Point quad[3], Point sub_quad1[3],
                    Point sub_quad2[3]) {
  Point p1 = (quad[0] + quad[1]) * 0.5f;
  Point p2 = (quad[1] + quad[2]) * 0.5f;
  Point p3 = (p1 + p2) * 0.5f;

  sub_quad1[0] = quad[0];
  sub_quad1[1] = p1;
  sub_quad1[2] = p3;

  sub_quad2[0] = p3;
  sub_quad2[1] = p2;
  sub_quad2[2] = quad[2];
}

void SubDividedQuad(const Vec2 quad[3], Vec2 sub_quad1[3], Vec2 sub_quad2[3]) {
  Vec2 p1 = (quad[0] + quad[1]) * 0.5f;
  Vec2 p2 = (quad[1] + quad[2]) * 0.5f;
  Vec2 p3 = (p1 + p2) * 0.5f;

  sub_quad1[0] = quad[0];
  sub_quad1[1] = p1;
  sub_quad1[2] = p3;

  sub_quad2[0] = p3;
  sub_quad2[1] = p2;
  sub_quad2[2] = quad[2];
}

void CubicToQuadratic(const Point cubic[4], Point quad[3]) {
  quad[0] = cubic[0];
  quad[1] = (3.f * (cubic[1] + cubic[2]) - (cubic[0] + cubic[3])) / 4.f;
  quad[2] = cubic[3];
}

void CalculateQuadPQ(Vec2 const& A, Vec2 const& B, Vec2 const& C, Vec2 const& P,
                     Vec2& out) {
  float a = -2.f * glm::dot(A, A);
  float b = -3.f * glm::dot(A, B);
  float c = 2.f * glm::dot(P, A) - 2.f * glm::dot(C, A) - glm::dot(B, B);
  float d = glm::dot(P, B) - glm::dot(C, B);

  out.x = (3.f * a * c - b * b) / (3.f * a * a);
  out.y = (2.f * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);
}

int32_t IntersectLineLine(Point const& p1, Point const& p2, Point const& p3,
                          Point const& p4, Point& result) {
  double mua, mub;
  double denom, numera, numberb;

  denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
  numera = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
  numberb = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);

  if (glm::abs(numera) < NearlyZero && glm::abs(numberb) < NearlyZero &&
      glm::abs(denom) < NearlyZero) {
    result = (p1 + p2) * 0.5f;

    return 2;  // lines coincide aka 180deg
  }

  if (glm::abs(denom) < NearlyZero) {
    result.x = 0;
    result.y = 0;
    result.z = 0;
    result.w = 0;
    return 0;  // lines are parallel
  }

  mua = numera / denom;
  mub = numberb / denom;
  result.x = p1.x + mua * (p2.x - p1.x);
  result.y = p1.y + mua * (p2.y - p1.y);

  bool out1 = mua < 0 || mua > 1;
  bool out2 = mub < 0 || mub > 1;

  if (out1 & out2) {
    return 5;  // the intersection lines outside both segments
  } else if (out1) {
    return 3;  // the intersection lines outside segment 1
  } else if (out2) {
    return 4;  // the intersection lines outside segment2
  } else {
    return 1;
  }
}

int32_t IntersectLineLine(Vec2 const& p1, Vec2 const& p2, Vec2 const& p3,
                          Vec2 const& p4, Vec2& result) {
  double mua, mub;
  double denom, numera, numberb;

  denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
  numera = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
  numberb = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);

  if (glm::abs(numera) < NearlyZero && glm::abs(numberb) < NearlyZero &&
      glm::abs(denom) < NearlyZero) {
    result = (p1 + p2) * 0.5f;

    return 2;  // lines coincide aka 180deg
  }

  if (glm::abs(denom) < NearlyZero) {
    result.x = 0;
    result.y = 0;
    return 0;  // lines are parallel
  }

  mua = numera / denom;
  mub = numberb / denom;
  result.x = p1.x + mua * (p2.x - p1.x);
  result.y = p1.y + mua * (p2.y - p1.y);

  bool out1 = mua < 0 || mua > 1;
  bool out2 = mub < 0 || mub > 1;

  if (out1 & out2) {
    return 5;  // the intersection lines outside both segments
  } else if (out1) {
    return 3;  // the intersection lines outside segment 1
  } else if (out2) {
    return 4;  // the intersection lines outside segment2
  } else {
    return 1;
  }
}

bool PointInTriangle(Point const& p, Point const& p0, Point const& p1,
                     Point const& p2) {
  // https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
  float dx = p.x - p2.x;
  float dy = p.y - p2.y;
  float dx21 = p2.x - p1.x;
  float dy12 = p1.y - p2.y;
  float D = dy12 * (p0.x - p2.x) + dx21 * (p0.y - p2.y);
  float s = dy12 * dx + dx21 * dy;
  float t = (p2.y - p0.y) * dx + (p0.x - p2.x) * dy;

  if (D < 0.f) return s <= 0.f && t <= 0 && (s + t) >= D;
  return s >= 0 && t >= 0 && (s + t) <= D;
}

}  // namespace skity
