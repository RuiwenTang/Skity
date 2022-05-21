#ifndef SKITY_SRC_RENDER_SW_SW_SUBPIXEL_HPP
#define SKITY_SRC_RENDER_SW_SW_SUBPIXEL_HPP

#include <cstdint>

namespace skity {

enum {
  PIXEL_BITS = 8,
  ONE_PIXEL = 1 << PIXEL_BITS,
  PIXEL_MAXK = ONE_PIXEL - 1,
};

template <class T>
int32_t sw_up_scale(T value) {
  static int32_t scale = 1 << (PIXEL_BITS);
  return static_cast<int32_t>(value * scale);
}

template <class T>
int32_t sw_down_scale(T value) {
  return static_cast<int32_t>(value) >> (PIXEL_BITS);
}

inline int32_t sw_trunc(int32_t x) { return x >> PIXEL_BITS; }

inline int32_t sw_sub_pixels(int32_t x) { return x << PIXEL_BITS; }

struct Span {
  int32_t x;
  int32_t y;
  int32_t len;
  int32_t cover;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_SW_SW_SUBPIXEL_HPP