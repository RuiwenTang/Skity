
#ifndef SKITY_SRC_SVG_SVG_NODE_HPP
#define SKITY_SRC_SVG_SVG_NODE_HPP

#include <memory>

#include "src/svg/svg_types.hpp"

namespace skity {

/**
 * @enum SVGTag
 * Represent svg tags, not all of them is supported.
 */
enum class SVGTag {
  kCircle,
  kClipPath,
  kDefs,
  kEllipse,
  kFeBlend,
  kFeColorMatrix,
  kFeComposite,
  kFeDiffuseLighting,
  kFeDisplacementMap,
  kFeDistantLight,
  kFeFlood,
  kFeGaussianBlur,
  kFeImage,
  kFeMorphology,
  kFeOffset,
  kFePointLight,
  kFeTurbulence,
  kFilter,
  kG,
  kImage,
  kLine,
  kLinearGradient,
  kMask,
  kPath,
  kPattern,
  kPolygon,
  kPolyline,
  kRadialGradient,
  kRect,
  kStop,
  kSvg,
  kText,
  kTextLiteral,
  kTextPath,
  kTSpan,
  kUse,
};

#define SVG_PRES_ATTR(attr_name, attr_type, attr_inherited) \
 private:                                                   \
  bool Set##attr_name()

class SVGNode {
 public:
 private:
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_NODE_HPP
