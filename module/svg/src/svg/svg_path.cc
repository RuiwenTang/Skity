#include "src/svg/svg_path.hpp"

#include <skity/render/canvas.hpp>

namespace skity {

SVGPath::SVGPath() : SVGShape(SVGTag::kPath), path_() {}

const char* SVGPath::TagName() const { return "path"; }

bool SVGPath::ParseAndSetAttribute(const char* name, const char* value) {
  return SVGShape::ParseAndSetAttribute(name, value);
}

void SVGPath::OnDraw(Canvas* canvas, const SVGLengthContext& ctx,
                     const Paint& paint, uint32_t) const {
  // TODO support other path fill type
  canvas->drawPath(path_, paint);
}

Path SVGPath::OnAsPath(SVGRenderContext const&) const {
  Path ret = path_;

  this->MapToParent(&ret);
  return ret;
}

void SVGPath::OnSetAttribute(SVGAttribute attr, const SVGValue& v) {
  switch (attr) {
    case SVGAttribute::kD:
      if (const auto* path = v.As<SVGPathValue>()) {
        path_ = static_cast<const Path&>(*path);
      }
      break;
    default:
      SVGShape::OnSetAttribute(attr, v);
  }
}

}  // namespace skity
