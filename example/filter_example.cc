#include <skity/skity.hpp>

#include "glad/glad.h"

void draw_filter(skity::Canvas* canvas) {
  auto filter = skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 4.f);

  skity::Paint paint;
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setStrokeWidth(18);
  paint.setColor(0xff4285F4);
  paint.setStrokeCap(skity::Paint::kRound_Cap);

  paint.setMaskFilter(filter);

  glm::ivec4 sizes;
  glGetIntegerv(GL_VIEWPORT, &sizes[0]);

  skity::Path path;
  path.moveTo(10, 10);
  path.quadTo(256, 64, 128, 128);
  path.quadTo(10, 192, 250, 250);

  canvas->drawPath(path, paint);

  // canvas->drawCircle(100, 200, 50, paint);

  paint.setStyle(skity::Paint::kFill_Style);
  paint.setMaskFilter(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 20.f));

  canvas->save();
  canvas->translate(50.f, 0.f);
  canvas->drawCircle(300, 100, 50, paint);
  canvas->restore();
}