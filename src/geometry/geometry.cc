
#include "src/geometry/geometry.hpp"

#include "src/geometry/conic.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

bool conic_in_line(Conic const& conic) { return quad_in_line(conic.pts); }

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

float FindCubicCusp(const Point src[4]) {
  // When the adjacent control point matches the end point, it behaves as if
  // the cubic has a cusp: there's a point of max curvature where the derivative
  // goes to zero. Ideally, this would be where t is zero or one, but math
  // error makes not so. It is not uncommon to create cubics this way; skip
  // them.
  if (src[0] == src[1]) {
    return -1;
  }

  if (src[2] == src[3]) {
    return -1;
  }

  // Cubics only have a cusp if the line segments formed by the control and end
  // points cross. Detect crossing if line ends are on opposite sides of plane
  // formed by the other line.
  std::array<Point, 4> cubic{src[0], src[1], src[2], src[3]};
  if (OnSameSide(cubic, 0, 2) || OnSameSide(cubic, 2, 0)) {
    return -1;
  }
  // Cubics may have multiple points of maximum curvature, although at most only
  // one is a cusp.
  std::array<float, 3> max_curvature;
  int roots = FindCubicMaxCurvature(src, max_curvature.data());
  for (int32_t index = 0; index < roots; index++) {
    float test_t = max_curvature[index];
    if (0 >= test_t || test_t >= 1) {
      continue;
    }
    // A cusp is at the max curvature, and also has a derivative close to zero.
    // Choose the 'close to zero' meaning by comparing the derivative length
    // with the overall cubic size.
    Vector d_pt = eval_cubic_derivative(src, test_t);
    float d_pt_magnitude = PointLengthSqd(d_pt);
    float precision = calc_cubic_precision(src);
    if (d_pt_magnitude < precision) {
      // All three max curvature t values may be close to the cusp;
      // return thie first one
      return test_t;
    }
  }

  return -1;
}

}  // namespace skity
