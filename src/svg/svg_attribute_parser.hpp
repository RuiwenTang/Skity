
#ifndef SKITY_SRC_SVG_SVG_ATTRIBUTE_PARSER_HPP
#define SKITY_SRC_SVG_SVG_ATTRIBUTE_PARSER_HPP

#include <vector>

#include "src/svg/svg_types.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class SVGAttributeParser {
 public:
  SVGAttributeParser(const char*);

  bool ParseInteger(SVGIntegerType* value);
  bool ParseViewBox(SVGViewBoxType* box);
  bool ParsePreserveAspectRatio(SVGPreserveAspectRatio* aspect_ratio);

  bool Parse(SVGIntegerType* v) { return ParseInteger(v); }

  template <typename T>
  using ParseResult = Lazy<T>;

  template <typename T>
  static ParseResult<T> Parse(const char* value) {
    ParseResult<T> result;
    T parsed_value;
    if (SVGAttributeParser(value).Parse(&parsed_value)) {
      result.Set(std::move(parsed_value));
    }
    return result;
  }



 private:
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_ATTRIBUTE_PARSER_HPP
