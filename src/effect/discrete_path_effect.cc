#include "src/effect/discrete_path_effect.hpp"

#include <skity/geometry/point.hpp>

#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/graphic/path_measure.hpp"

namespace skity {

static void Perterb(Point* p, Vector const& tangent, float scale) {
  Vector normal = tangent;
  PointRotateCCW(&normal);
  normal.w = 0.f;

  PointSetLength<false>(normal, normal.x, normal.y, scale);

  *p += normal;
}

/**
 * @class LCGRandom
 *	This is copied from SkRandom in skia source tree, and remove some
 *	methods for internal usage.
 */
class LCGRandom {
 public:
  LCGRandom(uint32_t seed) : fSeed(seed) {}

  /** Return the next pseudo random number expressed as a SkScalar
      in the range [-SK_Scalar1..SK_Scalar1).
  */
  float nextSScalar1() { return FixedToFloat(this->nextSFixed1()); }

 private:
  /** Return the next pseudo random number as an unsigned 32bit value.
   */
  uint32_t nextU() {
    uint32_t r = fSeed * kMul + kAdd;
    fSeed = r;
    return r;
  }

  /** Return the next pseudo random number as a signed 32bit value.
   */
  int32_t nextS() { return (int32_t)this->nextU(); }

  /** Return the next pseudo random number expressed as a signed SkFixed
   in the range [-SK_Fixed1..SK_Fixed1).
   */
  int32_t nextSFixed1() { return this->nextS() >> 15; }

  //  See "Numerical Recipes in C", 1992 page 284 for these constants
  enum { kMul = 1664525, kAdd = 1013904223 };
  uint32_t fSeed;
};

DiscretePathEffect::DiscretePathEffect(float seg_length, float deviation,
                                       uint32_t seed_assist)
    : PathEffect(),
      seg_length_(seg_length),
      perterb_(deviation),
      seed_assist_(seed_assist) {}

bool DiscretePathEffect::onFilterPath(Path* dst, Path const& src, bool stroke,
                                      Paint const&) const {
  bool do_fill = !stroke;

  PathMeasure meas{src, do_fill};

  uint32_t seed =
      seed_assist_ ^ static_cast<int32_t>(std::round(meas.getLength()));

  LCGRandom rand{seed ^ ((seed << 16) | (seed >> 16))};
  float scale = perterb_;
  Point p;
  Vector v;
  do {
    float length = meas.getLength();
    if (seg_length_ * (2 + static_cast<int32_t>(do_fill)) > length) {
      meas.getSegment(0, length, dst, true);
    } else {
      int32_t n = static_cast<int32_t>(std::round(length / seg_length_));
      constexpr int kMaxReasonableIterations = 100000;
      n = std::min(n, kMaxReasonableIterations);
      float delta = length / n;
      float distance = 0;

      if (meas.isClosed()) {
        n -= 1;
        distance += delta / 2;
      }

      if (meas.getPosTan(distance, &p, &v)) {
        Perterb(&p, v, rand.nextSScalar1() * scale);
        dst->moveTo(p);
      }

      while (--n >= 0) {
        distance += delta;
        if (meas.getPosTan(distance, &p, &v)) {
          Perterb(&p, v, rand.nextSScalar1() * scale);
          dst->lineTo(p);
        }
      }
      if (meas.isClosed()) {
        dst->close();
      }
    }
  } while (meas.nextContour());

  return true;
}

}  // namespace skity