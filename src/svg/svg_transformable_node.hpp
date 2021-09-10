#ifndef SKITY_SRC_SVG_SVG_TRANSFORMABLE_NODE_HPP
#define SKITY_SRC_SVG_SVG_TRANSFORMABLE_NODE_HPP

#include "src/svg/svg_node.hpp"

namespace skity {

class SVGTransformableNode : public SVGNode {
 public:
  ~SVGTransformableNode() override = default;

  void SetTransform(const SVGTransformType& t) { transform_ = t; }

  bool ParseAndSetAttribute(const char *name, const char *value) override;

 protected:
  explicit SVGTransformableNode(SVGTag);

  bool OnPrepareToRender(SVGRenderContext*) const override;

  void OnSetAttribute(SVGAttribute, const SVGValue&) override;

  void MapToParent(Path*) const;
  void MapToParent(Rect*) const;

 private:
  SVGTransformType transform_;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_TRANSFORMABLE_NODE_HPP
