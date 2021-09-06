#include "src/xml/xml_parser.hpp"

#include <cstring>

#include "pugixml.hpp"

namespace skity {

struct simple_walker : pugi::xml_tree_walker {
  XMLParser* parser = nullptr;

  explicit simple_walker(XMLParser* p) : parser(p) {}
  ~simple_walker() override = default;

  bool for_each(pugi::xml_node& node) override {
    if (!parser->StartElement(node.name())) {
      return false;
    }

    for (auto it = node.attributes_begin(); it != node.attributes_end(); it++) {
      if (!parser->AddAttribute(it->name(), it->value())) {
        return false;
      }
    }
    if (!node.text().empty()) {
      auto xml_text = node.text();
      auto str = xml_text.as_string();
      size_t len = std::strlen(str);

      if (!parser->Text(str, len)) {
        return false;
      }
    }

    if (!parser->EndElement(node.name())) {
      return false;
    }
    return true;
  }
};

static const char* const g_error_strings[] = {
    "empty or missing file ",    "unknown element ", "unknown attribute name ",
    "error in attribute value ", "duplicate ID ",    "unknown error "};

XMLParserError::XMLParserError()
    : code_(kNoError), line_number_(-1), native_code_(-1) {
  Reset();
}

std::string XMLParserError::GetErrorString() const {
  if (code_ != kNoError) {
    std::string ret;
    if (static_cast<uint32_t>(code_) < 6) {
      ret += g_error_strings[code_ - 1];
    }
    ret += noun_;
    return ret;
  } else {
    return "";
  }
}

void XMLParserError::Reset() {
  code_ = kNoError;
  line_number_ = -1;
  native_code_ = -1;
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_string("tree.xml");
}

XMLParser::XMLParser(XMLParserError* error) : error_(error) {}

bool XMLParser::Parse(const std::string& doc) {
  return Parse(doc.c_str(), doc.size());
}

bool XMLParser::Parse(const char* doc, size_t len) {
  pugi::xml_document document;
  pugi::xml_parse_result result = document.load_buffer(doc, len);
  if (!result) {
    error_->code_ = XMLParserError::kUnknownError;
    error_->SetNoun(result.description());
    return false;
  }

  simple_walker walker{this};
  document.traverse(walker);

  return error_->code_ == XMLParserError::kNoError;
}

bool XMLParser::StartElement(const char* elem) {
  return this->OnStartElement(elem);
}

bool XMLParser::AddAttribute(const char* name, const char* value) {
  return this->OnAddAttribute(name, value);
}

bool XMLParser::EndElement(const char* elem) {
  return this->OnEndElement(elem);
}

bool XMLParser::Text(const char* text, int32_t len) {
  return this->OnText(text, len);
}

bool XMLParser::OnStartElement(const char* elem) { return false; }
bool XMLParser::OnAddAttribute(const char* name, const char* value) {
  return false;
}
bool XMLParser::OnEndElement(const char* elem) { return false; }
bool XMLParser::OnText(const char* text, int32_t len) { return false; }

}  // namespace skity
