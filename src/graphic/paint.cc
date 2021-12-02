#include <skity/graphic/paint.hpp>
#include <skity/text/typeface.hpp>

namespace skity {

Paint::Paint() = default;

Paint::~Paint() = default;

Paint& Paint::operator=(const Paint& paint) = default;

void Paint::reset() { *this = Paint(); }

Paint::Style Paint::getStyle() const { return style_; }

void Paint::setStyle(Style style) {
  if (style > StyleCount) {
    return;
  }

  style_ = style;
}

void Paint::setStrokeWidth(float width) {
  stroke_width_ = width;
  updateMiterLimit();
}

float Paint::getStrokeWidth() const { return stroke_width_; }

float Paint::getStrokeMiter() const { return miter_limit_; }

void Paint::setStrokeMiter(float miter) { miter_limit_ = miter; }

Paint::Cap Paint::getStrokeCap() const { return cap_; }

void Paint::setStrokeCap(Cap cap) { cap_ = cap; }

Paint::Join Paint::getStrokeJoin() const { return join_; }

void Paint::setStrokeJoin(Join join) { join_ = join; }

void Paint::SetStrokeColor(float r, float g, float b, float a) {
  stroke_color_[0] = r;
  stroke_color_[1] = g;
  stroke_color_[2] = b;
  stroke_color_[3] = a;
}

Vector Paint::GetStrokeColor() const { return stroke_color_; }

void Paint::SetFillColor(float r, float g, float b, float a) {
  fill_color_[0] = r;
  fill_color_[1] = g;
  fill_color_[2] = b;
  fill_color_[3] = a;
}

Vector Paint::GetFillColor() const { return fill_color_; }

void Paint::setColor(Color color) {
  auto color4f = Color4fFromColor(color);
  stroke_color_ = color4f;
  fill_color_ = color4f;
}

void Paint::setAntiAlias(bool aa) { is_anti_alias_ = aa; }

bool Paint::isAntiAlias() const { return false; }

float Paint::getAlphaF() const { return global_alpha_; }

void Paint::setAlphaF(float a) {
  a = glm::clamp(a, 0.f, 1.f);
  global_alpha_ = a;
}

uint8_t Paint::getAlpha() const {
  return static_cast<uint8_t>(std::round(this->getAlphaF() * 255));
}

void Paint::setAlpha(uint8_t alpha) { this->setAlphaF(alpha * (1.f / 255)); }

void Paint::updateMiterLimit() {
  // FIXME: hard code miter limit
  miter_limit_ = 4.5f * stroke_width_ / 2.f;
}

}  // namespace skity
