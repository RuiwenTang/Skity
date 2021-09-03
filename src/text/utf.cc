#include <skity/text/utf.hpp>

namespace skity {

static constexpr inline int32_t left_shift(int32_t value, int32_t shift) {
  return static_cast<int32_t>(static_cast<uint32_t>(value) << shift);
}

template <typename T>
static constexpr bool is_align2(T x) {
  return 0 == (x & 1);
}

template <typename T>
static constexpr bool is_align4(T x) {
  return 0 == (x & 3);
}

template <typename T>
static int32_t next_fail(const T** ptr, const T* end) {
  *ptr = end;
  return -1;
}

static constexpr inline bool utf16_is_high_surrogate(uint16_t c) {
  return (c & 0xFC00) == 0xD800;
}

static constexpr inline bool utf16_is_low_surrogate(uint16_t c) {
  return (c & 0xFC00) == 0xDC00;
}

/** @returns   -1  iff invalid UTF8 byte,
                0  iff UTF8 continuation byte,
                1  iff ASCII byte,
                2  iff leading byte of 2-byte sequence,
                3  iff leading byte of 3-byte sequence, and
                4  iff leading byte of 4-byte sequence.
      I.e.: if return value > 0, then gives length of sequence.
*/
static int utf8_byte_type(uint8_t c) {
  if (c < 0x80) {
    return 1;
  } else if (c < 0xC0) {
    return 0;
  } else if (c >= 0xF5 ||
             (c & 0xFE) ==
                 0xC0) {  // "octet values c0, c1, f5 to ff never appear"
    return -1;
  } else {
    int value = (((0xe5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
    // assert(value >= 2 && value <=4);
    return value;
  }
}

static bool utf8_type_is_valid_leading_byte(int type) { return type > 0; }

static bool utf8_byte_is_continuation(uint8_t c) {
  return utf8_byte_type(c) == 0;
}

int32_t UTF::CountUTF8(const char* text, size_t byte_length) {
  if (!text) {
    return -1;
  }

  int32_t count = 0;
  const char* stop = text + byte_length;
  while (text < stop) {
    int type = utf8_byte_type(*(const uint8_t*)text);
    if (!utf8_type_is_valid_leading_byte(type) || text + type > stop) {
      // Sequence extends beyond end.
      return -1;
    }

    while (type-- > 1) {
      ++text;
      if (!utf8_byte_is_continuation(*(const uint8_t*)text)) {
        return -1;
      }
    }
    ++text;
    ++count;
  }

  return count;
}

int32_t UTF::NextUTF8(const char** ptr, const char* end) {
  if (!ptr || !end) {
    return -1;
  }

  const uint8_t* p = (const uint8_t*)*ptr;
  if (!p || p >= (const uint8_t*)end) {
    return next_fail(ptr, end);
  }

  int32_t c = *p;
  int32_t hic = c << 24;
  if (!utf8_type_is_valid_leading_byte(utf8_byte_type(c))) {
    return next_fail(ptr, end);
  }

  if (hic < 0) {
    uint32_t mask = (uint32_t)~0x3F;
    hic = left_shift(hic, 1);
    do {
      ++p;
      if (p >= (const uint8_t*)end) {
        return next_fail(ptr, end);
      }
      // check before reading off end of array
      uint8_t next_byte = *p;
      if (!utf8_byte_is_continuation(next_byte)) {
        return next_fail(ptr, end);
      }
      c = (c << 6) | (next_byte & 0x3F);
      mask <<= 5;
    } while ((hic = left_shift(hic, 1)) < 0);
    c &= ~mask;
  }
  *ptr = (char*)p + 1;
  return c;
}

bool UTF::UTF8ToCodePoint(const char* text, size_t byte_length,
                          std::vector<GlyphID>& glyph_ids) {
  int32_t count = CountUTF8(text, byte_length);
  if (count < 0) {
    return false;
  }

  glyph_ids.resize(count);
  const char* p = text;
  const char* end = text + byte_length;

  for (int32_t i = 0; i < count; i++) {
    GlyphID id = NextUTF8(&p, end);
    if (id < 0) {
      return false;
    }

    glyph_ids[i] = id;
  }

  return true;
}

}  // namespace skity
