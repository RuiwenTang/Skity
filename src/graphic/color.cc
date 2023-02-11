#include <cmath>
#include <glm/glm.hpp>
#include <skity/graphic/color.hpp>

namespace skity {

static float hsla_hue(float h, float m1, float m2) {
  if (h < 0) h += 1.f;
  if (h > 1) h -= 1.f;
  if (h < 1.f / 6.f) {
    return m1 + (m2 - m1) * h * 6.f;
  } else if (h < 3.f / 6.f) {
    return m2;
  } else if (h < 4.f / 6.f) {
    return m1 + (m2 - m1) * (2.f / 3.f - h) * 6.f;
  } else {
    return m1;
  }
}

Color ColorMakeFromHSLA(float h, float s, float l, uint8_t a) {
  float m1, m2;
  h = fmodf(h, 1.f);
  if (h < 0.f) h += 1.f;
  s = glm::clamp(s, 0.f, 1.f);
  l = glm::clamp(l, 0.f, 1.f);
  m2 = l <= 0.5f ? (l * (1.f + s)) : (l + s - l * s);
  m1 = 2.f * l - m2;

  auto r = static_cast<uint8_t>(
      glm::clamp(hsla_hue(h + 1.f / 3.f, m1, m2), 0.f, 1.f) * 255.f);
  auto g =
      static_cast<uint8_t>(glm::clamp(hsla_hue(h, m1, m2), 0.f, 1.f) * 255.f);
  auto b = static_cast<uint8_t>(
      glm::clamp(hsla_hue(h - 1.f / 3.f, m1, m2), 0.f, 1.f) * 255.f);

  return ColorSetARGB(a, r, g, b);
}

Color4f Color4fFromColor(Color color) {
  float r = ColorGetR(color) / 255.f;
  float g = ColorGetG(color) / 255.f;
  float b = ColorGetB(color) / 255.f;
  float a = ColorGetA(color) / 255.f;

  return Color4f{r, g, b, a};
}

}  // namespace skity
