#include "src/xml/xml_parser.hpp"

#include "pugixml.hpp"

namespace skity {

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

}  // namespace skity
