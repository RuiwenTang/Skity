
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

class SVGRect : public SVGShape {
 public:
  SVGRect() : SVGShape(SVGTag::kRect) {}
  ~SVGRect() override = default;

  SVG_ATTR(X, SVGLength, SVGLength(0))
  SVG_ATTR(Y, SVGLength, SVGLength(0))
  SVG_ATTR(Width, SVGLength, SVGLength(0))
  SVG_ATTR(Height, SVGLength, SVGLength(0))

  SVG_OPTIONAL_ATTR(Rx, SVGLength)
  SVG_OPTIONAL_ATTR(Ry, SVGLength)

  const char *TagName() const override { return "rect"; }

  bool ParseAndSetAttribute(const char *name, const char *value) override {
    return SVGNode::ParseAndSetAttribute(name, value) ||
           this->SetX(SVGAttributeParser::Parse<SVGLength>("x", name, value)) ||
           this->SetY(SVGAttributeParser::Parse<SVGLength>("y", name, value)) ||
           this->SetWidth(
               SVGAttributeParser::Parse<SVGLength>("width", name, value)) ||
           this->SetHeight(
               SVGAttributeParser::Parse<SVGLength>("height", name, value)) ||
           this->SetRx(
               SVGAttributeParser::Parse<SVGLength>("rx", name, value)) ||
           this->SetRy(SVGAttributeParser::Parse<SVGLength>("ry", name, value));
  }

 protected:
  void OnDraw(Canvas *canvas, const SVGLengthContext &ctx, const Paint &paint,
              uint32_t uint32) const override {
    canvas->drawRRect(this->Resolve(ctx), paint);
  }

  Path OnAsPath(const SVGRenderContext &context) const override {
    Path path;
    path.addRRect(this->Resolve(context.GetLengthContext()));

    MapToParent(&path);

    return path;
  }

  RRect Resolve(SVGLengthContext const &lctx) const {
    auto rect = lctx.ResolveRect(fX, fY, fWidth, fHeight);

    // https://www.w3.org/TR/SVG11/shapes.html#RectElementRXAttribute:
    //
    //   - Let rx and ry be length values.
    //   - If neither ‘rx’ nor ‘ry’ are properly specified, then set both rx and
    //   ry to 0.
    //   - Otherwise, if a properly specified value is provided for ‘rx’, but
    //   not for ‘ry’,
    //     then set both rx and ry to the value of ‘rx’.
    //   - Otherwise, if a properly specified value is provided for ‘ry’, but
    //   not for ‘rx’,
    //     then set both rx and ry to the value of ‘ry’.
    //   - Otherwise, both ‘rx’ and ‘ry’ were specified properly. Set rx to the
    //   value of ‘rx’
    //     and ry to the value of ‘ry’.
    //   - If rx is greater than half of ‘width’, then set rx to half of
    //   ‘width’.
    //   - If ry is greater than half of ‘height’, then set ry to half of
    //   ‘height’.
    //   - The effective values of ‘rx’ and ‘ry’ are rx and ry, respectively.
    auto radii = [this]() {
      return fRx.IsValid()   ? fRy.IsValid() ? std::make_tuple(*fRx, *fRy)
                                             : std::make_tuple(*fRx, *fRx)
               : fRy.IsValid() ? std::make_tuple(*fRy, *fRy)
                             : std::make_tuple(SVGLength{0}, SVGLength{0});
    };

    auto radius = radii();
    auto rx = std::min(lctx.Resolve(std::get<0>(radius),
                                    SVGLengthContext::LengthType::kHorizontal),
                       rect.width() / 2.f);
    auto ry = std::min(lctx.Resolve(std::get<1>(radius),
                                    SVGLengthContext::LengthType::kVertical),
                       rect.height() / 2.f);

    return RRect::MakeRectXY(rect, rx, ry);
  }
};

std::shared_ptr<SVGShape> SVGShape::Make(const char *name) {
  if (std::strcmp(name, "circle") == 0) {
    return std::make_shared<SVGCircle>();
  } else if (std::strcmp(name, "ellipse") == 0) {
    return std::make_shared<SVGEllipsis>();
  } else if (std::strcmp(name, "rect") == 0) {
    return std::make_shared<SVGRect>();
  }

  return nullptr;
}

}  // namespace skity
