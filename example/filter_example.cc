#include <skity/skity.hpp>

void draw_filter(skity::Canvas* canvas) {
  auto filter = skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 4.f);

  skity::Paint paint;
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setStrokeWidth(18);
  paint.setColor(0xff4285F4);
  paint.setStrokeCap(skity::Paint::kRound_Cap);

  paint.setMaskFilter(filter);

  skity::Path path;
  path.moveTo(10, 10);
  path.quadTo(256, 64, 128, 128);
  path.quadTo(10, 192, 250, 250);

  canvas->drawPath(path, paint);
}