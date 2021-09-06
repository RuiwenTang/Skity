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

  SVGProperty<SVGPaint, true> fill;
  SVGProperty<SVGNumberType, true> fill_opacity;
  // SVGProperty<SVGFillRule, true> fill_rule;
  // SVGProperty<SVGFillRule, true> clip_rule;

  SVGProperty<SVGPaint, true> stroke;
  SVGProperty<SVGDashArray, true> stroke_dash_array;
  SVGProperty<SVGLength, true> stroke_dash_offset;
  SVGProperty<SVGLineCap, true> stroke_line_cap;
  SVGProperty<SVGLineJoin, true> stroke_line_join;
  SVGProperty<SVGLength, true> stroke_miter_limit;
  SVGProperty<SVGNumberType, true> stroke_opacity;

  SVGProperty<SVGVisibility, true> visibility;

  SVGProperty<SVGColorType, true> color;

  // un inherited
  SVGProperty<SVGNumberType, false> opacity;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_ATTRIBUTE_HPP
