
#ifndef SKITY_SRC_SVG_SVG_NODE_HPP
#define SKITY_SRC_SVG_SVG_NODE_HPP

#include <memory>
#include <skity/geometry/point.hpp>
#include <skity/graphic/path.hpp>

#include "src/svg/svg_attribute.hpp"
#include "src/svg/svg_attribute_parser.hpp"
#include "src/svg/svg_types.hpp"

namespace skity {

class Paint;
class SVGLengthContext;
class SVGRenderContext;

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

#define SVG_PRES_ATTR(attr_name, attr_type, attr_inherited)               \
 private:                                                                 \
  bool Set##attr_name(SVGAttributeParser::ParseResult<                    \
                      SVGProperty<attr_type, attr_inherited>>&& pr) {     \
    if (pr.IsValid()) {                                                   \
      this->Set##attr_name(std::move(*pr));                               \
    }                                                                     \
    return pr.IsValid();                                                  \
  }                                                                       \
                                                                          \
 public:                                                                  \
  const SVGProperty<attr_type, attr_inherited>& Get##attr_name() const {  \
    return presentation_attributes_.f##attr_name;                         \
  }                                                                       \
  void Set##attr_name(const SVGProperty<attr_type, attr_inherited>& v) {  \
    auto* dest = &presentation_attributes_.f##attr_name;                  \
    if (!dest->IsInheritable() || v.IsValue()) {                          \
      *dest = v;                                                          \
    } else {                                                              \
      dest->Set(SVGPropertyState::kInherit);                              \
    }                                                                     \
  }                                                                       \
  void Set##attr_name(const SVGProperty<attr_type, attr_inherited>&& v) { \
    auto* dest = &presentation_attributes_.f##attr_name;                  \
    if (!dest->IsInheritable() || v.IsValue()) {                          \
      *dest = std::move(v);                                               \
    } else {                                                              \
      dest->Set(SVGPropertyState::kInherit);                              \
    }                                                                     \
  }

class SVGNode {
 public:
  virtual ~SVGNode() = default;

  SVGTag Tag() const { return tag_; }

  virtual void AppendChild(std::shared_ptr<SVGNode> child) = 0;

  void Render(const SVGRenderContext&) const;
  bool AsPaint(const SVGRenderContext&, Paint*) const;
  Path AsPath(const SVGRenderContext&) const;

  void SetAttribute(SVGAttribute, const SVGValue&);
  bool SetAttribute(const char* name, const char* value);
  bool ParseAndSetAttribute(const char* name, const char* value);

  // inherited
  SVG_PRES_ATTR(Color, SVGColorType, true)
  SVG_PRES_ATTR(Fill, SVGPaint, true)
  SVG_PRES_ATTR(FillOpacity, SVGNumberType, true)
  SVG_PRES_ATTR(Stroke, SVGPaint, true)
  SVG_PRES_ATTR(StrokeDashArray, SVGDashArray, true)
  SVG_PRES_ATTR(StrokeDashOffset, SVGLength, true)
  SVG_PRES_ATTR(StrokeLineCap, SVGLineCap, true)
  SVG_PRES_ATTR(StrokeLineJoin, SVGLineJoin, true)
  SVG_PRES_ATTR(StrokeMiterLimit, SVGNumberType, true)
  SVG_PRES_ATTR(StrokeOpacity, SVGNumberType, true)
  SVG_PRES_ATTR(StrokeWidth, SVGLength, true)

  // not inherited

 protected:
  explicit SVGNode(SVGTag tag);

  static Matrix ComputeViewBoxMatrix(Rect const&, Rect const&,
                                     SVGPreserveAspectRatio);

  virtual bool OnPrepareToRender(SVGRenderContext*) const;

  virtual void OnRender(SVGRenderContext const&) const = 0;
  virtual bool OnAsPaint(SVGRenderContext const&, Paint*) const {
    return false;
  }
  virtual Path OnAsPath(SVGRenderContext const&) const = 0;
  virtual bool HasChildren() const { return false; }
  virtual void OnSetAttribute(SVGAttribute, const SVGValue&) {}

 private:
  SVGTag tag_;
  SVGPresentationAttributes presentation_attributes_;
};

#undef SVG_PRES_ATTR

#define _SVG_ATTR_SETTERS(attr_name, attr_type, attr_default, set_cp, set_mv) \
 private:                                                                     \
  bool Set##attr_name(const SVGAttributeParser::ParseResult<attr_type>& pr) { \
    if (pr.IsValid()) {                                                       \
      this->Set##attr_name(*pr);                                              \
    }                                                                         \
    return pr.IsValid();                                                      \
  }                                                                           \
  bool Set##attr_name(SVGAttributeParser::ParseResult<attr_type>&& pr) {      \
    if (pr.IsValid()) {                                                       \
      this->Set##attr_name(std::move(*pr));                                   \
    }                                                                         \
    return pr.IsValid();                                                      \
  }                                                                           \
                                                                              \
 public:                                                                      \
  void Set##attr_name(const attr_type& a) { set_cp(a); }                      \
  void Set##attr_name(attr_type&& a) { set_mv(std::move(a)); }

#define SVG_ATTR(attr_name, attr_type, attr_default)               \
 private:                                                          \
  attr_type f##attr_name = attr_default;                           \
                                                                   \
 public:                                                           \
  const attr_type& Get##attr_name() const { return f##attr_name; } \
  _SVG_ATTR_SETTERS(                                               \
      attr_name, attr_type, attr_default,                          \
      [this](const attr_type& a) { this->f##attr_name = a; },      \
      [this](attr_type&& a) { this->f##attr_name = std::move(a); })

#define SVG_OPTIONAL_ATTR(attr_name, attr_type)                          \
 private:                                                                \
  Lazy<attr_type> f##attr_name;                                          \
                                                                         \
 public:                                                                 \
  const Lazy<attr_type>& Get##attr_name() const { return f##attr_name; } \
  _SVG_ATTR_SETTERS(                                                     \
      attr_name, attr_type, attr_default,                                \
      [this](const attr_type& a) { this->f##attr_name.Set(a); },         \
      [this](attr_type&& a) { this->f##attr_name.Set(std::move(a)); })

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_NODE_HPP
