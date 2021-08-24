#ifndef SKITY_GRAPHIC_COLOR_HPP
#define SKITY_GRAPHIC_COLOR_HPP

#include <cstdint>
#include <skity/geometry/point.hpp>

namespace skity {

/**
 * 32-bit ARGB color value, unpremultiplied.
 */
using Color = uint32_t;

/**
 * Returns color value from 8-bit component values.
 *
 * @param a amount of alpha, from fully transparent (0) to fully opaque (255)
 * @param r amount of red, from no red (0) to full red (255)
 * @param g amount of green, from no green (0) to full green (255)
 * @param b amount of blue, from no blue (0) to full blue (255)
 * @return  color and alpha, unpremultiplied
 */
static constexpr inline Color ColorSetARGB(uint8_t a, uint8_t r, uint8_t g,
                                           uint8_t b) {
  return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

/**
 * Returns color value from 8-bit component values, with alpha set fully opaque
 * to 255.
 */
#define ColorSetRGB(r, g, b) ColorSetARGB(0xFF, r, g, b)

/**
 * Returns alpha byte from color value.
 */
#define ColorGetA(color) (((color) >> 24) & 0xFF)

/**
 * Returns red component of color, from zero to 255.
 */
#define ColorGetR(color) (((color) >> 16) & 0xFF)

/**
 * Returns green component of color, from zero to 255.
 */
#define ColorGetG(color) (((color) >> 8) & 0xFF)

/**
 * Returns blue component of color, from zero to 255.
 */
#define ColorGetB(color) (((color) >> 0) & 0xFF)

/**
 * Returns unpremultiplied color with red, blue, and green set from c; and alpha
 * set from a. Alpha component of c is ignored and is replaced by a in result.
 * @param c
 * @param a
 * @return
 */
static constexpr inline Color ColorSetA(Color c, uint8_t a) {
  return (c & 0x00FFFFFF) | (a << 24);
}

constexpr uint8_t Color_AlphaTRANSPARENT = 0x00;
constexpr uint8_t Color_AlphaOPAQUE = 0xFF;

/**
 * Represents fully transparent Color.
 */
constexpr Color Color_TRANSPARENT = ColorSetARGB(0x0, 0x0, 0x0, 0x0);
/**
 * Represents fully opaque black.
 */
constexpr Color Color_BLACK = ColorSetARGB(0xFF, 0x0, 0x0, 0x0);
/**
 * Represents fully opaque dark gray.
 * @note SVG dark gray is equivalent to 0xFFA9A9A9.
 */
constexpr Color Color_DKGRAY = ColorSetARGB(0xFF, 0x44, 0x44, 0x44);
/**
 * Represents fully opaque gray.
 * @note HTML gray is equivalent to 0xFF808080.
 */
constexpr Color Color_GRAY = ColorSetARGB(0xFF, 0x88, 0x88, 0x88);
/**
 * Represents fully opaque light gray.
 * @note HTML silver is equivalent to 0xFFC0C0C0.
 *       SVG light gray is equivalent to 0xFFD3D3D3.
 */
constexpr Color Color_LTGRAY = ColorSetARGB(0xFF, 0xCC, 0xCC, 0xCC);
/**
 * Represents fully opaque white.
 */
constexpr Color Color_WHITE = ColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF);
/**
 * Represents fully opaque red.
 */
constexpr Color Color_RED = ColorSetARGB(0xFF, 0xFF, 0x00, 0x00);
/**
 * Represents fully opaque green. HTML lime is equivalent.
 * @note HTML green is equivalent to 0xFF008000.
 */
constexpr Color Color_GREEN = ColorSetARGB(0xFF, 0x00, 0xFF, 0x00);
/**
 * Represents fully opaque blue.
 */
constexpr Color Color_BLUE = ColorSetARGB(0xFF, 0x00, 0x00, 0xFF);
/**
 * Represents fully opaque yellow.
 */
constexpr Color Color_YELLOW = ColorSetARGB(0xFF, 0xFF, 0xFF, 0x00);
/**
 * Represents fully opaque cyan. HTML aqua is equivalent.
 */
constexpr Color Color_CYAN = ColorSetARGB(0xFF, 0x00, 0xFF, 0xFF);
/**
 * Represents fully opaque magenta.
 */
constexpr Color Color_MAGENTA = ColorSetARGB(0xFF, 0xFF, 0x00, 0xFF);

Color ColorMakeFromHSLA(float h, float s, float l, uint8_t a);

// RGBA color value, holding four floating point components
using Color4f = Vec4;

Color4f Color4fFromColor(Color color);

namespace Colors {

constexpr Color4f kTransparent = {0, 0, 0, 0};
constexpr Color4f kBlack = {0, 0, 0, 1.f};
constexpr Color4f kDkGray = {0.25f, 0.25f, 0.25f, 1.f};
constexpr Color4f kGray = {0.50f, 0.50f, 0.50f, 1};
constexpr Color4f kLtGray = {0.75f, 0.75f, 0.75f, 1};
constexpr Color4f kWhite = {1, 1, 1, 1};
constexpr Color4f kRed = {1, 0, 0, 1};
constexpr Color4f kGreen = {0, 1, 0, 1};
constexpr Color4f kBlue = {0, 0, 1, 1};
constexpr Color4f kYellow = {1, 1, 0, 1};
constexpr Color4f kCyan = {0, 1, 1, 1};
constexpr Color4f kMagenta = {1, 0, 1, 1};

}  // namespace Colors

}  // namespace skity

#endif  // SKITY_GRAPHIC_COLOR_HPP