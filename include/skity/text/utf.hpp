#ifndef SKITY_TEXT_UTF_HPP
#define SKITY_TEXT_UTF_HPP

#include <cstddef>
#include <cstdint>
#include <skity/macros.hpp>
#include <skity/text/typeface.hpp>
#include <vector>

namespace skity {

/**
 * @class UTF
 * utf8 convert helper
 */
class SK_API UTF final {
 public:
  UTF() = delete;
  ~UTF() = delete;

  /**
   * Given a sequence of UTF-8 bytes, return the number of unicode codepoints.
   *
   * @param utf8        pointer to utf8 string
   * @param byte_length	length of utf8 string
   * @return            number of unicode codepoint or -1 if invalid.
   */
  static int32_t CountUTF8(const char* text, size_t byte_length);

  /**
   * Given a sequence of aligned UTF-8 characters in machine-endian form, return
   * the first unicode codepoint.
   *
   * @param ptr
   * @param end
   * @return -1 if invalid.
   */
  static int32_t NextUTF8(const char** ptr, const char* end);

  static bool UTF8ToCodePoint(const char* text, size_t byte_length,
                              std::vector<GlyphID>& glyph_ids);
};

}  // namespace skity

#endif  // SKITY_TEXT_UTF_HPP
