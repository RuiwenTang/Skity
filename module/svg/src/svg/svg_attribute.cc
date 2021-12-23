
#include "src/svg/svg_attribute.hpp"

namespace skity {

SVGPresentationAttributes SVGPresentationAttributes::MakeInitial() {
  SVGPresentationAttributes result;

  result.fFill.Set(SVGPaint{SVGColor{Color_BLACK}});
  result.fFillOpacity.Set(SVGNumberType{1.f});

  result.fStroke.Set(SVGPaint{SVGPaint::Type::kNone});
  result.fStrokeDashArray.Set(SVGDashArray{SVGDashArray::Type::kNone});
  result.fStrokeDashOffset.Set(SVGLength{0});
  result.fStrokeLineCap.Set(SVGLineCap::kButt);
  result.fStrokeLineJoin.Set(SVGLineJoin{SVGLineJoin::Type::kMiter});
  result.fStrokeMiterLimit.Set(SVGNumberType{4.f});
  result.fStrokeOpacity.Set(SVGNumberType{1.f});
  result.fStrokeWidth.Set(SVGLength{1.f});

  result.fColor.Set(SVGColorType{Color_BLACK});

  return result;
}

}  // namespace skity
