#include "src/svg/svg_transformable_node.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <skity/render/canvas.hpp>

#include "src/svg/svg_render_context.hpp"

namespace skity {

SVGTransformableNode::SVGTransformableNode(SVGTag tag)
    : SVGNode(tag), transform_(glm::identity<Matrix>()) {}

bool SVGTransformableNode::OnPrepareToRender(SVGRenderContext* ctx) const {
  if (transform_ != glm::identity<Matrix>()) {
    ctx->SaveOnce();
    ctx->GetCanvas()->concat(transform_);
  }

  return SVGNode::OnPrepareToRender(ctx);
}

void SVGTransformableNode::OnSetAttribute(SVGAttribute attr,
                                          const SVGValue& value) {
  switch (attr) {
    case SVGAttribute::kTransform:
      if (const auto* transform = value.As<SVGTransformValue>()) {
        this->SetTransform(static_cast<const SVGTransformType&>(*transform));
      }
      break;
    default:
      SVGNode::OnSetAttribute(attr, value);
      break;
  }
}

void SVGTransformableNode::MapToParent(Path* path) const {
  // TODO implement
}

void SVGTransformableNode::MapToParent(Rect*) const {
  // TODO implement
}
bool SVGTransformableNode::ParseAndSetAttribute(const char* name,
                                                const char* value) {
  auto transform =
      SVGAttributeParser::Parse<SVGTransformType>("transform", name, value);
  if (transform.IsValid()) {
    transform_ = *transform;
    return true;
  }
  return SVGNode::ParseAndSetAttribute(name, value);
}

}  // namespace skity
