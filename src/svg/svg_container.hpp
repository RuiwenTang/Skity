
#ifndef SKITY_SRC_SVG_SVG_CONTAINER_HPP
#define SKITY_SRC_SVG_SVG_CONTAINER_HPP

#include <vector>

#include "src/svg/svg_transformable_node.hpp"

namespace skity {

class SVGContainer : public SVGTransformableNode {
 public:
  ~SVGContainer() override = default;
  void AppendChild(std::shared_ptr<SVGNode>) override;

 protected:
  explicit SVGContainer(SVGTag tag);

  void OnRender(const SVGRenderContext&) const override;

  Path OnAsPath(SVGRenderContext const&) const override;

  bool HasChildren() const final;

  std::vector<std::shared_ptr<SVGNode>> children_;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_CONTAINER_HPP
