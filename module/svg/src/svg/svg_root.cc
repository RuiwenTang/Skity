
#include "src/svg/svg_root.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <skity/render/canvas.hpp>

#include "src/svg/svg_render_context.hpp"

namespace skity {

bool SVGRoot::OnPrepareToRender(SVGRenderContext* ctx) const {
  auto viewPortRect =
      ctx->GetLengthContext().ResolveRect(fX, fY, fWidth, fHeight);
  auto contentMatrix = glm::translate(
      glm::identity<Matrix>(), {viewPortRect.x(), viewPortRect.y(), 0.f});
  auto viewPort = Vec2{viewPortRect.width(), viewPortRect.height()};

  if (fViewBox.IsValid()) {
    const Rect& viewBox = *fViewBox;
    // An empty viewBox disables rendering.
    if (viewBox.isEmpty()) {
      return false;
    }

    // a viewBox overrides the intrinsic viewport
    contentMatrix = contentMatrix * ComputeViewBoxMatrix(viewBox, viewPortRect,
                                                         fPreserveAspectRatio);
  }

  if (contentMatrix != glm::identity<Matrix>()) {
    ctx->SaveOnce();
    ctx->GetCanvas()->concat(contentMatrix);
  }

  if (viewPort != ctx->GetLengthContext().ViewPort()) {
    ctx->WritableLengthContext()->SetViewPort(viewPort);
  }



  return SVGContainer::OnPrepareToRender(ctx);
}

void SVGRoot::OnSetAttribute(SVGAttribute attr, const SVGValue& value) {
  switch (attr) {
    case SVGAttribute::kX:
      if (const auto* x = value.As<SVGLengthValue>()) {
        this->SetX(static_cast<const SVGLength&>(*x));
      }
      break;
    case SVGAttribute::kY:
      if (const auto* y = value.As<SVGLengthValue>()) {
        this->SetY(static_cast<const SVGLength&>(*y));
      }
      break;
    case SVGAttribute::kWidth:
      if (const auto* w = value.As<SVGLengthValue>()) {
        this->SetWidth(static_cast<const SVGLength&>(*w));
      }
      break;
    case SVGAttribute::kHeight:
      if (const auto* h = value.As<SVGLengthValue>()) {
        this->SetHeight(static_cast<const SVGLength&>(*h));
      }
      break;
    case SVGAttribute::kViewBox:
      if (const auto* vb = value.As<SVGViewBoxValue>()) {
        this->SetViewBox(static_cast<const SVGViewBoxType&>(*vb));
      }
      break;
    case SVGAttribute::kPreserveAspectRatio:
      if (const auto* par = value.As<SVGPreserveAspectRatioValue>()) {
        this->SetPreserveAspectRatio(
            static_cast<const SVGPreserveAspectRatio&>(*par));
      }
      break;
    default:
      SVGContainer::OnSetAttribute(attr, value);
      break;
  }
}

// https://www.w3.org/TR/SVG11/coords.html#IntrinsicSizing
Vec2 SVGRoot::IntrinsicSize(const SVGLengthContext& ctx) const {
  // percentage values do not provide an intrinsic size.
  if (fWidth.unit() == SVGLength::Unit::kPercentage ||
      fHeight.unit() == SVGLength::Unit::kPercentage) {
    return Vec2{0, 0};
  }

  return Vec2{ctx.Resolve(fWidth, SVGLengthContext::LengthType::kHorizontal),
              ctx.Resolve(fHeight, SVGLengthContext::LengthType::kVertical)};
}
const char* SVGRoot::TagName() const { return "svg"; }

bool SVGRoot::ParseAndSetAttribute(const char* name, const char* value) {
  // unused ignore attribute
  if (std::strcmp(name, "version") == 0) {
    return true;
  } else if (std::strcmp(name, "xmlns") == 0) {
    return true;
  }
  return SVGNode::ParseAndSetAttribute(name, value) ||
         this->SetWidth(
             SVGAttributeParser::Parse<SVGLength>("width", name, value)) ||
         this->SetHeight(
             SVGAttributeParser::Parse<SVGLength>("height", name, value));
}

}  // namespace skity
