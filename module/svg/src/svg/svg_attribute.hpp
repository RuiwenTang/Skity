#ifndef SKITY_SRC_SVG_SVG_ATTRIBUTE_HPP
#define SKITY_SRC_SVG_SVG_ATTRIBUTE_HPP

#include "src/svg/svg_types.hpp"

namespace skity {

class SVGRenderContext;

enum class SVGAttribute {
  kClipRule,
  kColor,
  kColorInterpolation,
  kColorInterpolationFilters,
  kCx,  // <circle>, <ellipse>, <radialGradient>: center x position
  kCy,  // <circle>, <ellipse>, <radialGradient>: center y position
  kD,   // path data
  kFill,
  kFillOpacity,
  kFillRule,
  kFilter,
  kFilterUnits,
  kFontFamily,
  kFontSize,
  kFontStyle,
  kFontWeight,
  kFx,  // <radialGradient>: focal point x position
  kFy,  // <radialGradient>: focal point y position
  kGradientUnits,
  kGradientTransform,
  kHeight,
  kHref,
  kOpacity,
  kPoints,
  kPreserveAspectRatio,
  kR,   // <circle>, <radialGradient>: radius
  kRx,  // <ellipse>,<rect>: horizontal (corner) radius
  kRy,  // <ellipse>,<rect>: vertical (corner) radius
  kSpreadMethod,
  kStroke,
  kStrokeDashArray,
  kStrokeDashOffset,
  kStrokeOpacity,
  kStrokeLineCap,
  kStrokeLineJoin,
  kStrokeMiterLimit,
  kStrokeWidth,
  kTransform,
  kText,
  kTextAnchor,
  kViewBox,
  kVisibility,
  kWidth,
  kX,
  kX1,  // <line>: first endpoint x
  kX2,  // <line>: second endpoint x
  kY,
  kY1,  // <line>: first endpoint y
  kY2,  // <line>: second endpoint y

  kUnknown,
};

struct SVGPresentationAttributes {
  static SVGPresentationAttributes MakeInitial();

  SVGProperty<SVGPaint, true> fFill;
  SVGProperty<SVGNumberType, true> fFillOpacity;
  // SVGProperty<SVGFillRule, true> fFill_rule;
  // SVGProperty<SVGFillRule, true> fClip_rule;

  SVGProperty<SVGPaint, true> fStroke;
  SVGProperty<SVGDashArray, true> fStrokeDashArray;
  SVGProperty<SVGLength, true> fStrokeDashOffset;
  SVGProperty<SVGLineCap, true> fStrokeLineCap;
  SVGProperty<SVGLineJoin, true> fStrokeLineJoin;
  SVGProperty<SVGNumberType, true> fStrokeMiterLimit;
  SVGProperty<SVGNumberType, true> fStrokeOpacity;
  SVGProperty<SVGLength, true> fStrokeWidth;

  SVGProperty<SVGVisibility, true> fVisibility;

  SVGProperty<SVGColorType, true> fColor;

  // un inherited
  SVGProperty<SVGNumberType, false> fOpacity;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_ATTRIBUTE_HPP
