
#ifndef SKITY_SRC_GEOMETRY_GEOMETRY_HPP_
#define SKITY_SRC_GEOMETRY_GEOMETRY_HPP_

#include <array>
#include <glm/ext/scalar_constants.hpp>
#include <skity/geometry/point.hpp>

#include "src/geometry/math.hpp"

namespace skity {

enum class RotationDirection {
  kCW,
  kCCW,
};

static inline glm::vec2 FromPoint(Point const& p) {
  return glm::vec2{p.x, p.y};
}

static inline Point ToPoint(glm::vec2 const& x) {
  return Point{x.x, x.y, 0, 1};
}

template <class P1, class P2>
bool PointEqualPoint(P1 const& p1, P2 const& p2) {
  return FloatNearlyZero(p1.x - p2.x) && FloatNearlyZero(p1.y - p2.y);
}

static inline int CollapsDuplicates(float array[], int count) {
  for (int n = count; n > 1; n--) {
    if (array[0] == array[1]) {
      for (int i = 1; i < n; i++) {
        array[i - 1] = array[i];
      }
      count -= 1;
    } else {
      array += 1;
    }
  }
  return count;
}

/**
 * use for : eval(t) = A * t ^ 2 + B * t + C
 */
struct QuadCoeff {
  glm::vec2 A{};
  glm::vec2 B{};
  glm::vec2 C{};

  QuadCoeff() = default;
  QuadCoeff(glm::vec2 const& a, glm::vec2 const& b, glm::vec2 const& c)
      : A{a}, B{b}, C{c} {}

  explicit QuadCoeff(std::array<Point, 3> const& src);

  Point evalAt(float t);

  glm::vec2 eval(float t);

  glm::vec2 eval(glm::vec2 const& tt);

  static Point EvalQuadAt(std::array<Point, 3> const& src, float t);

  static void EvalQuadAt(std::array<Point, 3> const& src, float t, Point* outP,
                         Vector* outTangent);

  static Vector EvalQuadTangentAt(std::array<Point, 3> const& src, float t);

  static void ChopQuadAt(const Point src[3], Point dst[5], float t);
};

/**
 * use for : eval(t) = A * t ^ 3 + B * t ^ 2 + C * t + D
 */
struct CubicCoeff {
  glm::vec2 A{};
  glm::vec2 B{};
  glm::vec2 C{};
  glm::vec2 D{};

  explicit CubicCoeff(std::array<Point, 4> const& src);

  Point evalAt(float t);

  glm::vec2 eval(float t);

  glm::vec2 eval(glm::vec2 const& t);

  static void EvalCubicAt(const Point src[4], float t, Point* loc,
                          Vector* tangent, Vector* curvature);

  static void ChopCubicAt(const Point src[4], Point dst[7], float t);
};

struct Conic;

struct ConicCoeff {
  explicit ConicCoeff(Conic const& conic);

  glm::vec2 eval(float t);

  QuadCoeff numer;
  QuadCoeff denom;
};

static inline int valid_unit_divide(float number, float denom, float* radio) {
  if (number < 0) {
    number = -number;
    denom = -denom;
  }

  if (denom == 0 || number == 0 || number >= denom) {
    return 0;
  }

  float r = number / denom;
  if (FloatIsNan(r)) {
    return 0;
  }

  if (r == 0) {
    return 0;
  }

  *radio = r;
  return 1;
}

static inline int return_check_zero(int value) {
  if (value == 0) {
    return 0;
  }
  return value;
}

static inline int FindUnitQuadRoots(float A, float B, float C, float roots[2]) {
  if (A == 0) {
    return return_check_zero(valid_unit_divide(-C, B, roots));
  }

  float* r = roots;
  double dr = (double)B * B - 4 * (double)A * C;
  if (dr < 0) {
    return return_check_zero(0);
  }

  dr = glm::sqrt(dr);
  float R = static_cast<float>(dr);
  if (glm::isinf(R)) {
    return return_check_zero(0);
  }

  float Q = (B < 0) ? -(B - R) / 2 : -(B + R) / 2;
  r += valid_unit_divide(Q, A, r);
  r += valid_unit_divide(C, Q, r);
  if (r - roots == 2) {
    if (roots[0] > roots[1]) {
      std::swap(roots[0], roots[1]);
    } else if (roots[0] == roots[1]) {
      r -= 1;
    }
  }

  return return_check_zero(static_cast<int>(r - roots));
}

/*  Solve coeff(t) == 0, returning the number of roots that
    lie withing 0 < t < 1.
    coeff[0]t^3 + coeff[1]t^2 + coeff[2]t + coeff[3]

    Eliminates repeated roots (so that all tValues are distinct, and are always
    in increasing order.
*/
inline static int solve_cubic_poly(const float coeff[4], float tValues[3]) {
  if (FloatNearlyZero(coeff[0])) {  // we're just a quadratic
    return FindUnitQuadRoots(coeff[1], coeff[2], coeff[3], tValues);
  }

  float a, b, c, Q, R;
  {
    float inva = FloatInvert(coeff[0]);
    a = coeff[1] * inva;
    b = coeff[2] * inva;
    c = coeff[3] * inva;
  }

  Q = (a * a - b * 3) / 9;
  R = (2 * a * a * a - 9 * a * b + 27 * c) / 54;

  float Q3 = Q * Q * Q;
  float R2MinusQ3 = R * R - Q3;
  float adiv3 = a / 3;

  if (R2MinusQ3 < 0) {
    // we have 3 real roots
    // the divide/root can, due to finite precisions, be slightly outside of
    // -1...1
    float theta = glm::acos(TPin(R / glm::sqrt(Q3), -1.0f, 1.0f));
    float neg2RootQ = -2 * glm::sqrt(Q);

    tValues[0] = TPin(neg2RootQ * glm::cos(theta / 3) - adiv3, 0.0f, 1.0f);
    tValues[1] =
        TPin(neg2RootQ * glm::cos((theta + 2 * glm::pi<float>()) / 3) - adiv3,
             0.0f, 1.0f);
    tValues[2] =
        TPin(neg2RootQ * glm::cos((theta - 2 * glm::pi<float>()) / 3) - adiv3,
             0.0f, 1.0f);

    BubbleSort(tValues, 3);
    return CollapsDuplicates(tValues, 3);
  } else {
    // we have 1 real root
    float A = glm::abs(R) + glm::sqrt(R2MinusQ3);
    A = CubeRoot(A);
    if (R > 0) {
      A = -A;
    }
    if (A != 0) {
      A += Q / A;
    }
    tValues[0] = TPin(A - adiv3, 0.0f, 1.0f);
    return 1;
  }
}

/*  Looking for F' dot F'' == 0

    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a

    F' = 3Ct^2 + 6Bt + 3A
    F'' = 6Ct + 6B

    F' dot F'' -> CCt^3 + 3BCt^2 + (2BB + CA)t + AB
*/
inline static void formulate_F1DotF2(const float src[], float coeff[4]) {
  float a = src[2] - src[0];
  float b = src[4] - 2 * src[2] + src[0];
  float c = src[6] + 3 * (src[2] - src[4]) - src[0];

  coeff[0] = c * c;
  coeff[1] = 3 * b * c;
  coeff[2] = 2 * b * b + c * a;
  coeff[3] = a * b;
}

bool DegenerateVector(Vector const& v);

/**
 * Returns the distance squared from the point to the line segment
 *
 * @param pt          point
 * @param lineStart   start point of the line
 * @param lineEnd     end point of the line
 *
 * @return distance squared value
 */
float pt_to_line(Point const& pt, Point const& lineStart, Point const& lineEnd);

void SubDividedCubic(const Point cubic[4], Point sub_cubic1[4],
                     Point sub_cubic2[4]);

void SubDividedCubic2(const Point cubic[4], Point sub_cubic[8]);

void SubDividedCubic4(const Point cubic[4], Point sub_cubic[16]);

void SubDividedCubic8(const Point cubic[4], Point sub_cubic[32]);

void SubDividedQuad(const Point quad[3], Point sub_quad1[3],
                    Point sub_quad2[3]);

void SubDividedQuad(const Vec2 quad[3], Vec2 sub_quad1[3], Vec2 sub_quad2[3]);

void CubicToQuadratic(const Point cubic[4], Point quad[3]);

void CalculateQuadPQ(Vec2 const& A, Vec2 const& B, Vec2 const& C, Vec2 const& P,
                     Vec2& out);

/**
 * @brief calculate intersect point of two line p1p2, p3p4
 *
 * @param p1        start point of line1
 * @param p2        end point of line1
 * @param p3        start point of line2
 * @param p4        end point of line2
 * @param result    result point
 * @return int32_t  2 means 180, 0 parallel
 */
int32_t IntersectLineLine(Point const& p1, Point const& p2, Point const& p3,
                          Point const& p4, Point& result);

int32_t IntersectLineLine(Vec2 const& p1, Vec2 const& p2, Vec2 const& p3,
                          Vec2 const& p4, Vec2& result);

bool PointInTriangle(Point const& p, Point const& p0, Point const& p1,
                     Point const& p2);

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_GEOMETRY_HPP_
