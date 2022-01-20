#ifndef SKITY_EFFECT_MASK_FILTER_HPP
#define SKITY_EFFECT_MASK_FILTER_HPP

#include <memory>
#include <skity/geometry/rect.hpp>
#include <skity/macros.hpp>

namespace skity {

enum BlurStyle : int {
  kNormal = 1,  // fuzzy inside and outside
  kSolid,       // solid inside, fuzzy outside
  kOuter,       // nothing inside, fuzzy outside
  kInner,       // fuzzy inside, nothing outside
};

class SK_API MaskFilter {
 public:
  MaskFilter() = default;
  ~MaskFilter() = default;

  BlurStyle blurStyle() const { return style_; }

  float blurRadius() const { return radius_; }

  /**
   * @brief
   *
   * @param src
   * @return Rect
   */
  Rect approximateFilteredBounds(Rect const& src) const;

  /**
   * Create a blur mask filter
   *
   * @param style BlurStyle to use
   * @param radius Radius of the Gaussian blur to apply. Must be > 0.
   *
   * @return blur mask instance
   */
  static std::shared_ptr<MaskFilter> MakeBlur(BlurStyle style, float radius);

 private:
  BlurStyle style_ = {};
  float radius_ = {};
};

}  // namespace skity

#endif  // SKITY_EFFECT_MASK_FILTER_HPP