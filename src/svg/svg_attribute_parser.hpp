
#ifndef SKITY_SRC_SVG_SVG_ATTRIBUTE_PARSER_HPP
#define SKITY_SRC_SVG_SVG_ATTRIBUTE_PARSER_HPP

#include <cstring>
#include <vector>

#include "src/svg/svg_types.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class SVGAttributeParser {
 public:
  explicit SVGAttributeParser(const char*);

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

  template <typename T>
  static ParseResult<T> Parse(const char* expected_name, const char* name,
                              const char* value) {
    if (!std::strcmp(name, expected_name)) {
      return Parse<T>(value);
    }

    return ParseResult<T>{};
  }

  template <typename PropertyT>
  static ParseResult<PropertyT> ParseProperty(const char* expected_name,
                                              const char* name,
                                              const char* value) {
    if (std::strcmp(name, expected_name) != 0) {
      return ParseResult<PropertyT>{};
    }

    if (!std::strcmp(value, "inherit")) {
      PropertyT result{SVGPropertyState::kInherit};
      return ParseResult<PropertyT>{&result};
    }

    auto pr = Parse<typename PropertyT::ValueT>(value);
    if (pr.IsValid()) {
      PropertyT result{*pr};
      return ParseResult<PropertyT>{&result};
    }

    return ParseResult<PropertyT>{};
  }

  void* operator new(size_t) = delete;
  void* operator new(size_t, void*) = delete;

 private:
  template <class T>
  bool Parse(T*);

  template <class F>
  bool AdvanceWhile(F func);

  bool MatchStringToken(const char* token,
                        const char** new_pos = nullptr) const;

  bool ParseWSToken();
  bool ParseEOSToken();
  bool ParseSepToken();
  bool ParseCommaWspToken();
  bool ParseExpectedStringToken(const char*);
  bool ParseScalarToken(float*);
  bool ParseInt32Token(int32_t*);
  bool ParseHexToken(uint32_t*);
  bool ParseLengthUnitToken(SVGLength::Unit*);
  bool ParseNamedColorToken(Color*);
  bool ParseHexColorToken(Color*);
  bool ParseColorComponentToken(int32_t*);
  bool ParseRGBColorToken(Color*);
  bool ParseFuncIRI(void*);

  template <typename Func, typename T>
  bool ParseParenthesized(const char* prefix, Func, T* result);

  template <typename T>
  bool ParseList(std::vector<T>*);

 private:
  const char* cur_pos;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_SVG_ATTRIBUTE_PARSER_HPP
