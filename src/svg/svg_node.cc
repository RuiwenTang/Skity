
#include "src/svg/svg_node.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/svg/svg_render_context.hpp"

namespace skity {

SVGNode::SVGNode(SVGTag tag) : tag_(tag) {}

void SVGNode::Render(const SVGRenderContext &ctx) const {
  SVGRenderContext local_context{ctx};

  if (this->OnPrepareToRender(&local_context)) {
    this->OnRender(local_context);
  }
}

bool SVGNode::AsPaint(const SVGRenderContext &ctx, Paint *paint) const {
  SVGRenderContext local_context{ctx};

  return this->OnPrepareToRender(&local_context) &&
         this->OnAsPaint(local_context, paint);
}

Path SVGNode::AsPath(const SVGRenderContext &ctx) const {
  SVGRenderContext local_context{ctx};
  if (!this->OnPrepareToRender(&local_context)) {
    return Path{};
  }

  Path path = this->OnAsPath(local_context);

  if (const auto *clip_path = local_context.ClipPath()) {
    // TODO handle clip path
  }

  return path;
}

bool SVGNode::OnPrepareToRender(SVGRenderContext *ctx) const {
  ctx->ApplyPresentationAttribute(
      presentation_attributes_,
      this->HasChildren() ? 0 : SVGRenderContext::kLeaf);

  return true;
}

void SVGNode::SetAttribute(SVGAttribute attr, const SVGValue &v) {
  this->OnSetAttribute(attr, v);
}

bool SVGNode::SetAttribute(const char *name, const char *value) {
  return this->ParseAndSetAttribute(name, value);
}

template <typename T>
void SetInheritedByDefault(Lazy<T> &presentation_attr, const T &value) {
  if (value.type() != T::Type::kInherit) {
    presentation_attr.Set(value);
  } else {
    // kInherited
    presentation_attr.Reset();
  }
}

bool SVGNode::ParseAndSetAttribute(const char *name, const char *value) {
#define PARSE_AND_SET(svgName, attrName)                               \
  this->Set##attrName(SVGAttributeParser::ParseProperty<               \
                      decltype(presentation_attributes_.f##attrName)>( \
      svgName, name, value))

  return PARSE_AND_SET("color", Color) || PARSE_AND_SET("fill", Fill) ||
         PARSE_AND_SET("fill-opacity", FillOpacity) ||
         PARSE_AND_SET("stroke", Stroke) ||
         PARSE_AND_SET("stroke-dasharray", StrokeDashArray) ||
         PARSE_AND_SET("stroke-linecap", StrokeLineCap) ||
         PARSE_AND_SET("stroke-linejoin", StrokeLineCap) ||
         PARSE_AND_SET("stroke-miterlimit", StrokeMiterLimit) ||
         PARSE_AND_SET("stroke-opacity", StrokeOpacity) ||
         PARSE_AND_SET("stroke-width", StrokeWidth);

#undef PARSE_AND_SET
}

// https://www.w3.org/TR/SVG11/coords.html#PreserveAspectRatioAttribute
Matrix SVGNode::ComputeViewBoxMatrix(const Rect &viewBox, const Rect &viewPort,
                                     SVGPreserveAspectRatio par) {
  auto compute_scale = [&]() -> Vec2 {
    float sx = viewPort.width() / viewBox.width();
    float sy = viewPort.height() / viewBox.height();

    if (par.align == SVGPreserveAspectRatio::kNone) {
      return {sx, sy};
    }

    float s = par.scale == SVGPreserveAspectRatio::kMeet ? std::min(sx, sy)
                                                         : std::max(sx, sy);

    return {s, s};
  };

  auto compute_trans = [&](const Vec2 &scale) -> Vec2 {
    static constexpr float gAlignCoeffs[] = {
        0.0f,  // Min
        0.5f,  // Mid
        1.f,   // Max
    };

    size_t x_coeff = par.align >> 0 & 0x03;
    size_t y_coeff = par.align >> 2 & 0x03;

    float tx = -viewBox.x() * scale.x;
    float ty = -viewBox.y() * scale.y;
    float dx = viewPort.width() - viewBox.width() * scale.x;
    float dy = viewPort.height() - viewBox.height() * scale.y;

    return {tx + dx * gAlignCoeffs[x_coeff], ty + dy * gAlignCoeffs[y_coeff]};
  };

  Vec2 scale = compute_scale();
  Vec2 translate = compute_trans(scale);

  return glm::translate(glm::identity<Matrix>(),
                        {translate.x, translate.y, 0}) *
         glm::scale(glm::identity<Matrix>(), {scale.x, scale.y, 1.f});
}

}  // namespace skity
