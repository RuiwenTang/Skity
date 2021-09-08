
#ifndef SKITY_SRC_SVG_SVG_RENDER_CONTEXT_HPP
#define SKITY_SRC_SVG_SVG_RENDER_CONTEXT_HPP

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

#include "src/svg/svg_attribute.hpp"
#include "src/svg/svg_types.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class Canvas;
class SVGLength;

class SVGLengthContext {
 public:
  enum class LengthType {
    kHorizontal,
    kVertical,
    kOther,
  };

  explicit SVGLengthContext(Vec2 const& view_port, float dpi = 90.f)
      : view_port_(view_port), dpi_(dpi) {}

  Vec2 const& ViewPort() const { return view_port_; }
  void SetViewPort(Vec2 const& view_port) { view_port_ = view_port; }

  float Resolve(SVGLength const&, LengthType) const;

  Rect ResolveRect(SVGLength const& x, SVGLength const& y, SVGLength const& w,
                   SVGLength const& h) const;

 private:
  glm::vec2 view_port_;
  float dpi_;
};

struct SVGPresentationContext {
  SVGPresentationContext();
  SVGPresentationAttributes inherited;
};

class SVGRenderContext {
 public:
  SVGRenderContext(Canvas*, const SVGLengthContext&,
                   const SVGPresentationContext&);
  SVGRenderContext(SVGRenderContext const&);
  ~SVGRenderContext();

  void ApplyOpacity(float opacity, uint32_t flags, bool has_filter);

  Lazy<Paint> CommonPaint(const SVGPaint&, float opacity) const;

  SVGLengthContext const& GetLengthContext() const { return length_context_; }
  SVGLengthContext* WritableLengthContext() { return &length_context_; }

  Canvas* GetCanvas() const { return canvas_; }

  enum ApplyFlags {
    kLeaf = 1 << 0,
  };

  void ApplyPresentationAttribute(SVGPresentationAttributes const&,
                                  uint32_t flags);

  const Path* ClipPath() const { return clip_path_.GetMaybeNull(); }

  Lazy<Paint> FillPaint() const;
  Lazy<Paint> StrokePaint() const;
  SVGColorType ResolveSvgColor(SVGColor const&) const;

 private:
  Canvas* canvas_;
  int canvas_save_count_;
  Lazy<Path> clip_path_;
  float deferred_paint_opacity_ = 1.f;
  SVGLengthContext length_context_;
  SVGPresentationContext presentation_context_;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_RENDER_CONTEXT_HPP
