
#include "src/svg/svg_node.hpp"

namespace skity {

SVGNode::SVGNode(SVGTag tag) : tag_(tag) {}

void SVGNode::Render(const SVGRenderContext &) const {}

bool SVGNode::OnPrepareToRender(SVGRenderContext *) const { return false; }

}  // namespace skity
