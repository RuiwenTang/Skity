
#ifndef SKITY_SRC_GEOMETRY_GEOMETRY_HPP_
#define SKITY_SRC_GEOMETRY_GEOMETRY_HPP_

#include <array>
#include <glm/ext/scalar_constants.hpp>
#include <skity/geometry/point.hpp>

#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"

namespace skity {

enum class RotationDirection {
  kCW,
  kCCW,
};

static inline glm::vec2 FromPoint(Point const& p)
{
  return glm::vec2{p.x, p.y};
}

static inline Point ToPoint(glm::vec2 const& x)
{
  return Point{x.x, x.y, 0, 1};
}

static inline bool OnSameSide(std::array<Point, 4> const& src, int testIndex,
                              int lineIndex)
{
  Point origin = src[lineIndex];
  Vector line = src[lineIndex + 1] - origin;
  float crosses[2];
  for (int index = 0; index < 2; index++) {
    Vector testLine = src[testIndex + index] - origin;
    crosses[index] = CrossProduct(line, testLine);
  }
  return crosses[0] * crosses[1] >= 0;
}

static inline int CollapsDuplicates(float array[], int count)
{
  for (int n = count; n > 1; n--) {
    if (array[0] == array[1]) {
      for (int i = 1; i < n; i++) {
        array[i - 1] = array[i];
      }
      count -= 1;
    }
    else {
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
      : A{a}, B{b}, C{c}
  {
  }

  explicit QuadCoeff(std::array<Point, 3> const& src);

  Point evalAt(float t);

  glm::vec2 eval(float t);

  glm::vec2 eval(glm::vec2 const& tt);

  static Point EvalQuadAt(std::array<Point, 3> const& src, float t);

  static void EvalQuadAt(std::array<Point, 3> const& src, float t, Point* outP,
                         Vector* outTangent);

  static Vector EvalQuadTangentAt(std::array<Point, 3> const& src, float t);
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

static inline int return_check_zero(int value)
{
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
  double dr = (double)B * B - 4 * (double) A * C;
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

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_GEOMETRY_HPP_

