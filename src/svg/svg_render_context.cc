
#include "src/svg/svg_render_context.hpp"

#include <cassert>
#include <cstring>
#include <skity/effect/path_effect.hpp>
#include <skity/render/canvas.hpp>

#include "src/geometry/math.hpp"

namespace skity {

static float length_size_for_type(Vec2 const &view_port,
                                  SVGLengthContext::LengthType type) {
  switch (type) {
    case SVGLengthContext::LengthType::kHorizontal:
      return view_port.x;
    case SVGLengthContext::LengthType::kVertical:
      return view_port.y;
    case SVGLengthContext::LengthType::kOther: {
      // https://www.w3.org/TR/SVG11/coords.html#Units_viewport_percentage
      constexpr float r_sqrt2 = 1.f / FloatSqrt2;
      float w = view_port.x;
      float h = view_port.y;
      return r_sqrt2 * glm::sqrt(w * w + h * h);
    }
  }
  return 0;
}

static constexpr float kINMultiplier = 1.f;
static constexpr float kPTMultiplier = kINMultiplier / 72.272f;
static constexpr float kPCMultiplier = kPTMultiplier * 12;
static constexpr float kMMMultiplier = kINMultiplier / 25.4f;
static constexpr float kCMMultiplier = kMMMultiplier * 10;

float SVGLengthContext::Resolve(const SVGLength &len, LengthType type) const {
  switch (len.unit()) {
    case SVGLength::Unit::kNumber:
      // Fall through.
    case SVGLength::Unit::kPX:
      return len.Value();
    case SVGLength::Unit::kPercentage:
      return len.Value() * length_size_for_type(view_port_, type) / 100.f;
    case SVGLength::Unit::kCM:
      return len.Value() * dpi_ * kCMMultiplier;
    case SVGLength::Unit::kMM:
      return len.Value() * dpi_ * kMMMultiplier;
    case SVGLength::Unit::kIN:
      return len.Value() * dpi_ * kINMultiplier;
    case SVGLength::Unit::kPT:
      return len.Value() * dpi_ * kPTMultiplier;
    case SVGLength::Unit::kPC:
      return len.Value() * dpi_ * kPCMultiplier;
    default:
      return 0;
  }
}

static Paint::Cap to_cap(SVGLineCap const &cap) {
  switch (cap) {
    case SVGLineCap::kButt:
      return Paint::kButt_Cap;
    case SVGLineCap::kRound:
      return Paint::kRound_Cap;
    case SVGLineCap::kSquare:
      return Paint::kSquare_Cap;
  }
}

static Paint::Join to_join(SVGLineJoin const &join) {
  switch (join.type()) {
    case SVGLineJoin::Type::kMiter:
      return Paint::kMiter_Join;
    case SVGLineJoin::Type::kRound:
      return Paint::kRound_Join;
    case SVGLineJoin::Type::kBevel:
      return Paint::kBevel_Join;
    default:
      break;
  }
  return Paint::kDefault_Join;
}

static std::shared_ptr<PathEffect> dash_effect(
    SVGPresentationAttributes const &props, SVGLengthContext const &lctx) {
  if (props.fStrokeDashArray->type() != SVGDashArray::Type::kDashArray) {
    return nullptr;
  }

  const auto &da = *props.fStrokeDashArray;
  auto count = da.DashArray().size();
  std::vector<float> intervals(count);
  for (int32_t i = 0; i < count; i++) {
    intervals[i] =
        lctx.Resolve(da.DashArray()[i], SVGLengthContext::LengthType::kOther);
  }

  if (count & 1) {
    // If an odd number of values is provided, then the list of values is
    // repeated to yield an even number of values.
    intervals.resize(count * 2);
    std::memcpy(intervals.data() + count, intervals.data(),
                count * sizeof(float));
  }

  auto phase = lctx.Resolve(*props.fStrokeDashOffset,
                            SVGLengthContext::LengthType::kOther);

  return PathEffect::MakeDashPathEffect(intervals.data(), intervals.size(),
                                        phase);
}

Rect SVGLengthContext::ResolveRect(const SVGLength &x, const SVGLength &y,
                                   const SVGLength &w,
                                   const SVGLength &h) const {
  return Rect::MakeXYWH(this->Resolve(x, LengthType::kHorizontal),
                        this->Resolve(y, LengthType::kVertical),
                        this->Resolve(w, LengthType::kHorizontal),
                        this->Resolve(h, LengthType::kVertical));
}

SVGPresentationContext::SVGPresentationContext()
    : inherited(SVGPresentationAttributes::MakeInitial()) {}

SVGRenderContext::SVGRenderContext(Canvas *canvas, const SVGLengthContext &lctx,
                                   const SVGPresentationContext &pctx)
    : canvas_(canvas),
      canvas_save_count_(canvas->getSaveCount()),
      length_context_(lctx),
      presentation_context_(pctx) {}

SVGRenderContext::SVGRenderContext(const SVGRenderContext &other)
    : SVGRenderContext(other.canvas_, other.length_context_,
                       other.presentation_context_) {}

SVGRenderContext::~SVGRenderContext() {
  canvas_->restoreToCount(canvas_save_count_);
}

void SVGRenderContext::ApplyOpacity(float opacity, uint32_t flags,
                                    bool has_filter) {
  if (opacity >= 1.f) {
    return;
  }

  const auto &props = presentation_context_.inherited;
  bool has_fill = props.fFill->type() != SVGPaint::Type::kNone;
  bool has_stroke = props.fStroke->type() != SVGPaint::Type::kNone;

  if ((flags & kLeaf) && (has_fill ^ has_stroke) && !has_filter) {
    deferred_paint_opacity_ *= opacity;
  } else {
    Paint opacity_paint;
    opacity_paint.setAlphaF(glm::clamp(opacity, 0.f, 1.f));
    // TODO canvas save layer
  }
}

Lazy<Paint> SVGRenderContext::CommonPaint(const SVGPaint &paint_selector,
                                          float opacity) const {
  if (paint_selector.type() == SVGPaint::Type::kNone) {
    return Lazy<Paint>{};
  }

  Lazy<Paint> p;
  p.Init();
  p->setAntiAlias(true);
  switch (paint_selector.type()) {
    case SVGPaint::Type::kColor:
      p->setColor(this->ResolveSvgColor(paint_selector.color()));
      break;
    case SVGPaint::Type::kIRI:
      break;
    default:
      break;
  }

  p->setAlphaF(glm::clamp(opacity * deferred_paint_opacity_, 0.f, 1.f));

  return p;
}

void SVGRenderContext::ApplyPresentationAttribute(
    const SVGPresentationAttributes &attrs, uint32_t flags) {
#define ApplyLazyInheritedAttribute(ATTR)                                      \
  do {                                                                         \
    assert(presentation_context_.inherited.f##ATTR.IsValue());                 \
    const auto &attr = attrs.f##ATTR;                                          \
    if (attr.IsValue() && *attr != *presentation_context_.inherited.f##ATTR) { \
      presentation_context_.inherited.f##ATTR.Set(*attr);                      \
    }                                                                          \
  } while (false)

  ApplyLazyInheritedAttribute(Fill);
  ApplyLazyInheritedAttribute(FillOpacity);
  ApplyLazyInheritedAttribute(Stroke);
  ApplyLazyInheritedAttribute(StrokeDashOffset);
  ApplyLazyInheritedAttribute(StrokeDashArray);
  ApplyLazyInheritedAttribute(StrokeLineCap);
  ApplyLazyInheritedAttribute(StrokeLineJoin);
  ApplyLazyInheritedAttribute(StrokeMiterLimit);
  ApplyLazyInheritedAttribute(StrokeOpacity);
  ApplyLazyInheritedAttribute(StrokeWidth);
  // ApplyLazyInheritedAttribute(Visibility)
  ApplyLazyInheritedAttribute(Color);

#undef ApplyLazyInheritedAttribute

  bool has_filter = attrs.fFill.IsValue();
  if (attrs.fOpacity.IsValue()) {
    this->ApplyOpacity(*attrs.fOpacity, flags, has_filter);
  }
  // TODO apply clip
  // TODO apply mask

  if (has_filter) {
    // TODO apply filter
  }
}

void SVGRenderContext::SaveOnce() {
  if (canvas_->getSaveCount() == canvas_save_count_) {
    canvas_->save();
  }

  assert(canvas_->getSaveCount() > canvas_save_count_);
}

Lazy<Paint> SVGRenderContext::FillPaint() const {
  const auto &props = presentation_context_.inherited;
  auto p = this->CommonPaint(*props.fFill, *props.fFillOpacity);
  if (p.IsValid()) {
    p->setStyle(Paint::kFill_Style);
  }
  return p;
}

Lazy<Paint> SVGRenderContext::StrokePaint() const {
  const auto &props = presentation_context_.inherited;
  auto p = this->CommonPaint(*props.fStroke, *props.fStrokeOpacity);
  if (p.IsValid()) {
    p->setStyle(Paint::kStroke_Style);
    p->setStrokeWidth(length_context_.Resolve(
        *props.fStrokeWidth, SVGLengthContext::LengthType::kOther));
    p->setStrokeCap(to_cap(*props.fStrokeLineCap));
    p->setStrokeJoin(to_join(*props.fStrokeLineJoin));
    p->setStrokeMiter(*props.fStrokeMiterLimit);
    p->setPathEffect(dash_effect(props, length_context_));
  }
  return p;
}

SVGColorType SVGRenderContext::ResolveSvgColor(const SVGColor &color) const {
  switch (color.type()) {
    case SVGColor::Type::kColor:
      return color.Color();
    case SVGColor::Type::kCurrentColor:
      return *presentation_context_.inherited.fColor;
    case SVGColor::Type::kICCColor:
      // TODO implement ICC color
      return Color_BLACK;
  }
}

}  // namespace skity