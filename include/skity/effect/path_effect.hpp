#ifndef SKITY_EFFECT_PATH_EFFECT_HPP
#define SKITY_EFFECT_PATH_EFFECT_HPP

#include <memory>
#include <skity/macros.hpp>

namespace skity {

class Path;
class Paint;

/**
 * @class PathEffect
 *	PathEffect is the base class for objects in the SkPaint that affect the
 *	geometry of a drawing primitive before it is transformed by the canvas'
 *	matrix and drawn.
 *
 * 	Dashing is implemented as a subclass of SkPathEffect.
 */
class SK_API PathEffect {
 public:
  virtual ~PathEffect() = default;

  /**
   * Given a src path(input), apply this effect to src path, returning the new
   * path in dst, and return true. If this effect cannot be applied, return
   * false.
   *
   * @param dst			output of this effect
   * @param src			input of this effect
   * @param stroke	specify if path need stroke
   * @param paint       current paint for drawing this path
   * @return true		this effect can be applied
   * @return false	this effect cannot be applied
   */
  bool filterPath(Path* dst, Path const& src, bool stroke,
                  Paint const& paint) const;

  /**
   * If the PathEffect can be represented as a dash pattern, asADash will return
   * kDash and None otherwise. If a non NULL info is passed in, the various
   * DashInfo will be filled in if the PathEffect can be a dash pattern.
   *
   */
  enum class DashType {
    // ignores the info parameter
    kNone,
    // fills in all of the info parameter
    kDash,
  };

  struct DashInfo {
    DashInfo() : intervals(nullptr), count(0), phase(0) {}
    DashInfo(float* inv, int32_t c, float p)
        : intervals(inv), count(c), phase(p) {}

    // length of on/off intervals for dashed lines
    float* intervals;
    // number of intervals in the dash. should be even number
    int32_t count;
    // offset into the dashed interval pattern
    // mode the sum of all intervals
    float phase;
  };

  DashType asADash(DashInfo* info) const;

  /**
   * Create DiscretePathEffect.
   *	This path effect chops a path into discrete segments, and randomly
   *	displaces them.
   *
   * @param seg_length
   * @param dev
   * @param seed_assist
   * @return std::shared_ptr<PathEffect>
   */
  static std::shared_ptr<PathEffect> MakeDiscretePathEffect(
      float seg_length, float dev, uint32_t seed_assist = 0);

  static std::shared_ptr<PathEffect> MakeDashPathEffect(const float intervals[],
                                                        int count, float phase);

 protected:
  PathEffect() = default;

  virtual bool onFilterPath(Path*, Path const&, bool, Paint const&) const = 0;

  virtual DashType onAsADash(DashInfo*) const { return DashType::kNone; }
};

}  // namespace skity

#endif  // SKITY_EFFECT_PATH_EFFECT_HPP