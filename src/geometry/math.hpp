#ifndef SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_
#define SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_

#include <algorithm>
#include <glm/glm.hpp>
#include <limits>

namespace skity {

#define Float1 1.0f
#define FloatHalf 0.5f
#define FloatNaN std::numeric_limits<float>::quiet_NaN()
#define FloatInfinity std::numeric_limits<float>::infinity()
#define NearlyZero (Float1 / (1 << 12))
#define FloatRoot2Over2 0.707106781f

static inline bool FloatNearlyZero(float x, float tolerance = NearlyZero) {
  return glm::abs(x) <= tolerance;
}

static inline float CubeRoot(float x) { return glm::pow(x, 0.3333333f); }

static inline bool FloatIsNan(float x) { return x != x; }

[[clang::no_sanitize("float-divide-by-zero")]] static inline float
SkityIEEEFloatDivided(float number, float denom) {
  return number / denom;
}

#define FloatInvert(x) SkityIEEEFloatDivided(Float1, (x))

static inline bool FloatIsFinite(float x) { return !glm::isinf(x); }

static inline float CrossProduct(glm::vec4 const& a, glm::vec4 const& b) {
  return a.x * b.y - a.y * b.x;
}

static inline float DotProduct(glm::vec4 const& a, glm::vec4 const& b) {
  return a.x * b.x + a.y * b.y;
}

static inline glm::vec2 Times2(glm::vec2 const& value) { return value + value; }

template <class T>
T Interp(T const& v0, T const& v1, T const& t) {
  return v0 + (v1 - v0) * t;
}

enum class Orientation {
  kLinear,
  kClockWise,
  kAntiClockWise,
};

template <class T>
Orientation CalculateOrientation(T const& p, T const& q, T const& r) {
  int32_t val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (FloatNearlyZero(val)) {
    return Orientation::kLinear;
  }

  return (val > 0) ? Orientation::kClockWise : Orientation::kAntiClockWise;
}

template <class V>
Orientation CalculateOrientation(V const& v1, V const& v2) {
  int32_t val = v1.x * v2.y - v1.y * v2.x;
  if (FloatNearlyZero(val)) {
    return Orientation::kLinear;
  }

  return (val > 0) ? Orientation::kClockWise : Orientation::kAntiClockWise;
}

template <typename T>
T TPin(T const& value, T const& min, T const& max) {
  return value < min ? min : (value < max ? value : max);
}

template <typename T>
void BubbleSort(T array[], int count) {
  for (int i = count - 1; i > 0; i--) {
    for (int j = i; j > 0; j--) {
      if (array[j] < array[j - 1]) {
        std::swap(array[j], array[j - 1]);
      }
    }
  }
}

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_
