#ifndef SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_
#define SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <limits>

namespace skity {

#define Float1 1.0f
#define FloatHalf 0.5f
#define FloatNaN std::numeric_limits<float>::quiet_NaN()
#define FloatInfinity std::numeric_limits<float>::infinity()
#define NearlyZero (Float1 / (1 << 12))
#define FloatRoot2Over2 0.707106781f
#define FloatSqrt2 1.41421356f

#define FixedToFloat(x) ((x)*1.52587890625e-5f)

static inline bool FloatNearlyZero(float x, float tolerance = NearlyZero) {
  return glm::abs(x) <= tolerance;
}

static inline float FloatInterp(float A, float B, float t) {
  return A + (B - A) * t;
}

static void P3DInterp(const float src[7], float dst[7], float t) {
  float ab = FloatInterp(src[0], src[3], t);
  float bc = FloatInterp(src[3], src[6], t);
  dst[0] = ab;
  dst[3] = FloatInterp(ab, bc, t);
  dst[6] = bc;
}

static inline float SkityFloatHalf(float v) { return v * FloatHalf; }

static inline float CubeRoot(float x) { return glm::pow(x, 0.3333333f); }

static inline bool FloatIsNan(float x) { return x != x; }

[[clang::no_sanitize("float-divide-by-zero")]] static inline float
SkityIEEEFloatDivided(float number, float denom) {
  return number / denom;
}

#define FloatInvert(x) SkityIEEEFloatDivided(Float1, (x))

static inline bool FloatIsFinite(float x) { return !glm::isinf(x); }

static inline float FloatSinSnapToZero(float radians) {
  float v = std::sin(radians);
  return FloatNearlyZero(v) ? 0.f : v;
}

static inline float FloatCosSnapToZero(float radians) {
  float v = std::cos(radians);
  return FloatNearlyZero(v) ? 0.f : v;
}

static inline float FloatCopySign(float v1, float v2) {
  return std::copysignf(v1, v2);
}

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
  float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (FloatNearlyZero(val, 0.001f)) {
    return Orientation::kLinear;
  }

  return (val > 0) ? Orientation::kClockWise : Orientation::kAntiClockWise;
}

template <class T>
int32_t CrossProductResult(T const& p, T const& q, T const& r) {
  return (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
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
