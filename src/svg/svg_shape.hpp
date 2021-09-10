
#ifndef SKITY_SRC_SVG_SVG_SHAPE_HPP
#define SKITY_SRC_SVG_SVG_SHAPE_HPP

#include "src/svg/svg_transformable_node.hpp"

namespace skity {

class Canvas;

class SVGShape : public SVGTransformableNode {
 public:
  ~SVGShape() override = default;

  void AppendChild(std::shared_ptr<SVGNode> child) override;

  static std::shared_ptr<SVGShape> Make(const char* name);

 protected:
  explicit SVGShape(SVGTag);

  void OnRender(const SVGRenderContext&) const final;

  virtual void OnDraw(Canvas* canvas, const SVGLengthContext& ctx,
                      const Paint& paint, uint32_t) const = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_SHAPE_HPP
