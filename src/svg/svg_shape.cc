
#include "src/svg/svg_shape.hpp"

#include <cstring>
#include <skity/render/canvas.hpp>

#include "src/svg/svg_render_context.hpp"

namespace skity {

SVGShape::SVGShape(SVGTag tag) : SVGTransformableNode(tag) {}

void SVGShape::OnRender(const SVGRenderContext &ctx) const {
  const auto fillPaint = ctx.FillPaint();
  const auto strokePaint = ctx.StrokePaint();

  if (fillPaint.IsValid()) {
    this->OnDraw(ctx.GetCanvas(), ctx.GetLengthContext(), *fillPaint, 0);
  }

  if (strokePaint.IsValid()) {
    this->OnDraw(ctx.GetCanvas(), ctx.GetLengthContext(), *strokePaint, 0);
  }
}

void SVGShape::AppendChild(std::shared_ptr<SVGNode> child) {
  // SVG shape can not have children
}

class SVGCircle : public SVGShape {
 public:
  SVGCircle() : SVGShape(SVGTag::kCircle) {}
  ~SVGCircle() override = default;

  SVG_ATTR(Cx, SVGLength, SVGLength(0))
  SVG_ATTR(Cy, SVGLength, SVGLength(0))
  SVG_ATTR(R, SVGLength, SVGLength(0))

  bool ParseAndSetAttribute(const char *name, const char *value) override {
    return SVGNode::ParseAndSetAttribute(name, value) ||
           this->SetCx(
               SVGAttributeParser::Parse<SVGLength>("cx", name, value)) ||
           this->SetCy(
               SVGAttributeParser::Parse<SVGLength>("cy", name, value)) ||
           this->SetR(SVGAttributeParser::Parse<SVGLength>("r", name, value));
  }

  const char *TagName() const override { return "circle"; }

 protected:
  Path OnAsPath(const SVGRenderContext &ctx) const override {
    float cx = ctx.GetLengthContext().Resolve(
        fCx, SVGLengthContext::LengthType::kHorizontal);
    float cy = ctx.GetLengthContext().Resolve(
        fCy, SVGLengthContext::LengthType::kVertical);
    float r = ctx.GetLengthContext().Resolve(
        fR, SVGLengthContext::LengthType::kOther);

    Path path;
    path.addCircle(cx, cy, r);

    this->MapToParent(&path);
    return path;
  }

  void OnDraw(Canvas *canvas, const SVGLengthContext &ctx, const Paint &paint,
              uint32_t uint32) const override {
    float cx = ctx.Resolve(fCx, SVGLengthContext::LengthType::kHorizontal);
    float cy = ctx.Resolve(fCy, SVGLengthContext::LengthType::kVertical);
    float r = ctx.Resolve(fR, SVGLengthContext::LengthType::kOther);

    if (r > 0) {
      canvas->drawCircle(cx, cy, r, paint);
    }
  }
};

class SVGEllipsis : public SVGShape {
 public:
  SVGEllipsis() : SVGShape(SVGTag::kEllipse) {}
  ~SVGEllipsis() override = default;

  SVG_ATTR(Cx, SVGLength, SVGLength(0))
  SVG_ATTR(Cy, SVGLength, SVGLength(0))
  SVG_ATTR(Rx, SVGLength, SVGLength(0))
  SVG_ATTR(Ry, SVGLength, SVGLength(0))

  bool ParseAndSetAttribute(const char *name, const char *value) override {
    return SVGNode::ParseAndSetAttribute(name, value) ||
           this->SetCx(
               SVGAttributeParser::Parse<SVGLength>("cx", name, value)) ||
           this->SetCy(
               SVGAttributeParser::Parse<SVGLength>("cy", name, value)) ||
           this->SetRx(
               SVGAttributeParser::Parse<SVGLength>("rx", name, value)) ||
           this->SetRy(SVGAttributeParser::Parse<SVGLength>("ry", name, value));
  }

  const char *TagName() const override { return "ellipsis"; }

 protected:
  void OnDraw(Canvas *canvas, const SVGLengthContext &ctx, const Paint &paint,
              uint32_t uint32) const override {
    float cx = ctx.Resolve(fCx, SVGLengthContext::LengthType::kHorizontal);
    float cy = ctx.Resolve(fCy, SVGLengthContext::LengthType::kVertical);
    float rx = ctx.Resolve(fRx, SVGLengthContext::LengthType::kHorizontal);
    float ry = ctx.Resolve(fRy, SVGLengthContext::LengthType::kVertical);

    if (rx > 0 && ry > 0) {
      canvas->drawOval(Rect::MakeXYWH(cx - rx, cy - ry, rx * 2.f, ry * 2.f),
                       paint);
    }
  }

  Path OnAsPath(const SVGRenderContext &ctx) const override {
    float cx = ctx.GetLengthContext().Resolve(
        fCx, SVGLengthContext::LengthType::kHorizontal);
    float cy = ctx.GetLengthContext().Resolve(
        fCy, SVGLengthContext::LengthType::kVertical);
    float rx = ctx.GetLengthContext().Resolve(
        fRx, SVGLengthContext::LengthType::kHorizontal);
    float ry = ctx.GetLengthContext().Resolve(
        fRy, SVGLengthContext::LengthType::kVertical);

    Path path;
    path.addOval(Rect::MakeXYWH(cx - rx, cy - ry, rx * 2.f, ry * 2.f));

    this->MapToParent(&path);

    return path;
  }
};

std::shared_ptr<SVGShape> SVGShape::Make(const char *name) {
  if (std::strcmp(name, "circle") == 0) {
    return std::make_shared<SVGCircle>();
  } else if (std::strcmp(name, "ellipse") == 0) {
    return std::make_shared<SVGEllipsis>();
  }

  return nullptr;
}

}  // namespace skity
