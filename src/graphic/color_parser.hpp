#ifndef SKITY_SRC_GRAPHIC_COLOR_PARSER_HPP
#define SKITY_SRC_GRAPHIC_COLOR_PARSER_HPP

#include <skity/graphic/color.hpp>

namespace skity {

class ColorParser {
 public:
  static const char* FindNamedColor(const char* str, size_t len, Color* color);
};

}  // namespace skity

#endif  // SKITY_SRC_GRAPHIC_COLOR_PARSER_HPP
