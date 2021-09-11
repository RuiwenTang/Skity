#include <skity/codec/data.hpp>
#include <skity/svg/svg_dom.hpp>

#include "src/svg/svg_container.hpp"
#include "src/svg/svg_render_context.hpp"
#include "src/svg/svg_root.hpp"
#include "src/svg/svg_shape.hpp"
#include "src/xml/xml_parser.hpp"

namespace skity {

static bool parse_and_set_view_box(SVGNode *node, SVGAttribute attr,
                                   const char *value) {
  SVGViewBoxType view_box;
  SVGAttributeParser parser{value};
  if (!parser.ParseViewBox(&view_box)) {
    return false;
  }

  node->SetAttribute(attr, SVGViewBoxValue{view_box});
  return true;
}

class SVGDomParser : public XMLParser {
 public:
  explicit SVGDomParser(XMLParserError *err) : XMLParser(err) {}
  ~SVGDomParser() override = default;

  std::shared_ptr<SVGRoot> GetRootNode() const { return root_; }

 protected:
  bool OnStartElement(const char *elem) override {
    if (std::strcmp(elem, "svg") == 0) {
      root_ = SVGRoot::Make(SVGRoot::Type::kRoot);
      nodes_.emplace_back(root_);
      return true;
    }

    std::shared_ptr<SVGNode> node = nullptr;
    // shape
    if (!node) {
      node = SVGShape::Make(elem);
    }

    // g
    if (!node) {
      node = SVGG::Make(elem);
    }

    if (!node) {
      return false;
    }

    CurrentNode()->AppendChild(node);
    PushCurrent(node);
    return true;
  }

  bool OnAddAttribute(const char *name, const char *value) override {
    // common attribute
    if (std::strcmp(name, "viewBox") == 0) {
      if (parse_and_set_view_box(CurrentNode(), SVGAttribute::kViewBox,
                                 value)) {
        return true;
      }
    }

    return CurrentNode()->ParseAndSetAttribute(name, value);
  }
  bool OnEndElement(const char *elem) override {
    bool expect = std::strcmp(CurrentNode()->TagName(), elem) == 0;
    if (expect) {
      PopBackNode();
    }
    return expect;
  }
  bool OnText(const char *text, int32_t len) override {
    // TODO implement text render
    return XMLParser::OnText(text, len);
  }

 private:
  void PopBackNode() { nodes_.pop_back(); }

  SVGNode *CurrentNode() { return nodes_.back().get(); }

  void PushCurrent(std::shared_ptr<SVGNode> node) {
    nodes_.emplace_back(std::move(node));
  }

 private:
  std::vector<std::shared_ptr<SVGNode>> nodes_;
  std::shared_ptr<SVGRoot> root_;
};

SVGDom::~SVGDom() = default;

std::unique_ptr<SVGDom> SVGDom::MakeFromFile(const char *file) {
  auto data = Data::MakeFromFileName(file);
  if (!data) {
    return nullptr;
  }

  return MakeFromData(data.get());
}

std::unique_ptr<SVGDom> SVGDom::MakeFromString(const std::string &content) {
  return MakeFromMemory(content.c_str(), content.size());
}

std::unique_ptr<SVGDom> SVGDom::MakeFromData(const Data *data) {
  if (!data || data->IsEmpty()) {
    return nullptr;
  }

  return MakeFromMemory((const char *)data->RawData(), data->Size());
}

std::unique_ptr<SVGDom> SVGDom::MakeFromMemory(const char *data, size_t len) {
  if (!data || len == 0) {
    return nullptr;
  }

  XMLParserError error;

  SVGDomParser dom_parser{&error};

  if (!dom_parser.Parse(data, len)) {
    return nullptr;
  }

  std::unique_ptr<SVGDom> dom{new SVGDom(dom_parser.GetRootNode())};

  return dom;
}

void SVGDom::Render(Canvas *canvas) {
  if (!root_) {
    return;
  }
  Vec2 container_size = root_->IntrinsicSize(SVGLengthContext{Vec2{0, 0}});

  SVGLengthContext lctx{container_size};
  SVGPresentationContext pctx;
  SVGRenderContext context{canvas, lctx, pctx};

  root_->Render(context);
}

SVGDom::SVGDom(std::shared_ptr<SVGRoot> root) : root_(std::move(root)) {}

}  // namespace skity