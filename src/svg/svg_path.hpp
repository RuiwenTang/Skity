#ifndef SKITY_SRC_SVG_SVG_PATH_HPP
#define SKITY_SRC_SVG_SVG_PATH_HPP

#include <skity/graphic/path.hpp>

#include "src/svg/svg_shape.hpp"

namespace skity {

class SVGPath : public SVGShape {
 public:
  SVGPath();

  ~SVGPath() override = default;

  const char* TagName() const override;

  bool ParseAndSetAttribute(const char* name, const char* value) override;

 protected:
  void OnDraw(Canvas* canvas, const SVGLengthContext& ctx, const Paint& paint,
              uint32_t) const override;

  Path OnAsPath(SVGRenderContext const&) const override;

  void OnSetAttribute(SVGAttribute, const SVGValue&) override;

 private:
  mutable Path path_;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_PATH_HPP
