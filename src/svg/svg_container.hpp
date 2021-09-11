
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

class SVGG : public SVGContainer {
 public:
  SVGG() : SVGContainer(SVGTag::kG) {}
  ~SVGG() override = default;

  const char* TagName() const override;

  static std::shared_ptr<SVGG> Make(const char*);
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_CONTAINER_HPP
