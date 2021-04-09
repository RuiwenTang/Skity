
#ifndef SKITY_INCLUDE_SKITY_GEOMETRY_GEOMETRY_HPP_
#define SKITY_INCLUDE_SKITY_GEOMETRY_GEOMETRY_HPP_

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

static inline glm::vec2 FromPoint(Point const& p) {
  return glm::vec2{p.x, p.y};
}

static inline Point ToPoint(glm::vec2 const& x) {
  return Point{x.x, x.y, 0, 1};
}

static inline bool OnSameSide(std::array<Point, 4> const& src, int testIndex,
                              int lineIndex) {
  Point origin = src[lineIndex];
  Vector line = src[lineIndex + 1] - origin;
  float crosses[2];
  for (int index = 0; index < 2; index++) {
    Vector testLine = src[testIndex + index] - origin;
    crosses[index] = CrossProduct(line, testLine);
  }
  return crosses[0] * crosses[1] >= 0;
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
};

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GEOMETRY_GEOMETRY_HPP_