#include <skity/graphic/paint.hpp>

namespace skity {

Paint::Paint() = default;

Paint::~Paint() = default;

void Paint::reset()
{
  // TODO reset all properties;
}

Paint::Style Paint::getStyle() const { return style_; }

void Paint::setStyle(Style style)
{
  if (style > StyleCount) {
    return;
  }

  style_ = style;
}

void Paint::setStrokeWidth(float width) { stroke_width_ = width; }

float Paint::getStrokeWidth() const { return stroke_width_; }

float Paint::getStrokeMiter() const { return miter_limit_; }

void Paint::setStrokeMiter(float miter) { miter_limit_ = miter; }

Paint::Cap Paint::getStrokeCap() const { return cap_; }

void Paint::setStrokeCap(Cap cap) { cap_ = cap; }

Paint::Join Paint::getStrokeJoin() const { return join_; }

void Paint::setStrokeJoin(Join join) { join_ = join; }
}  // namespace skity

