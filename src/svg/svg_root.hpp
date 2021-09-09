
#ifndef SKITY_SRC_SVG_SVG_ROOT_HPP
#define SKITY_SRC_SVG_SVG_ROOT_HPP

#include <memory>

#include "src/svg/svg_container.hpp"

namespace skity {

class SVGRoot : public SVGContainer {
 public:
  enum class Type {
    kRoot,
    kInner,
  };

  ~SVGRoot() override = default;

  static std::shared_ptr<SVGRoot> Make(Type t = Type::kInner) {
    return std::shared_ptr<SVGRoot>(new SVGRoot{t});
  }

  SVG_ATTR(X, SVGLength, SVGLength(0))
  SVG_ATTR(Y, SVGLength, SVGLength(0))
  SVG_ATTR(Width, SVGLength, SVGLength(100, SVGLength::Unit::kPercentage))
  SVG_ATTR(Height, SVGLength, SVGLength(100, SVGLength::Unit::kPercentage))
  SVG_ATTR(PreserveAspectRatio, SVGPreserveAspectRatio,
           SVGPreserveAspectRatio())

  SVG_OPTIONAL_ATTR(ViewBox, SVGViewBoxType)

  Vec2 IntrinsicSize(const SVGLengthContext&) const;

 protected:
  bool OnPrepareToRender(SVGRenderContext*) const override;
  void OnSetAttribute(SVGAttribute, const SVGValue&) override;

 private:
  explicit SVGRoot(Type t) : SVGContainer(SVGTag::kSvg), type_(t) {}

 private:
  Type type_;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_ROOT_HPP
