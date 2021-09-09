#include <skity/codec/data.hpp>
#include <skity/svg/svg_dom.hpp>

#include "src/svg/svg_root.hpp"
#include "src/svg/svg_shape.hpp"
#include "src/xml/xml_parser.hpp"

namespace skity {

class SVGDomParser : public XMLParser {
 public:
  SVGDomParser(const std::shared_ptr<SVGNode> &root, XMLParserError *err)
      : XMLParser(err) {
    nodes_.emplace_back(root);
  }
  ~SVGDomParser() override = default;

 protected:
  bool OnStartElement(const char *elem) override {
    // shape
    auto node = SVGShape::Make(elem);

    if (!node) {
      return false;
    }

    CurrentNode()->AppendChild(node);
    PushCurrent(node);
    return true;
  }

  bool OnAddAttribute(const char *name, const char *value) override {
    return XMLParser::OnAddAttribute(name, value);
  }
  bool OnEndElement(const char *elem) override {
    bool expect = std::strcmp(CurrentNode()->TagName(), elem) == 0;
    if (expect) {
      PopBackNode();
    }
    return expect;
  }
  bool OnText(const char *text, int32_t len) override {
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
  std::shared_ptr<SVGRoot> svg_root = SVGRoot::Make(SVGRoot::Type::kRoot);

  SVGDomParser dom_parser{svg_root, &error};

  if (!dom_parser.Parse(data, len)) {
    return nullptr;
  }

  std::unique_ptr<SVGDom> dom{new SVGDom(svg_root)};

  return dom;
}

}  // namespace skity