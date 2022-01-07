#include <skity/skity.hpp>

float degree = 0.f;

void draw_clip_demo(skity::Canvas* canvas) {
  degree += 0.2f;
  skity::Paint clip_paint;
  clip_paint.setStyle(skity::Paint::kStroke_Style);
  clip_paint.setStrokeWidth(2.f);

  skity::Paint stroke_paint;
  stroke_paint.setStyle(skity::Paint::kStroke_Style);
  stroke_paint.setStrokeWidth(5.f);
  stroke_paint.setColor(skity::Color_RED);

  skity::Rect rect1 = skity::Rect::MakeXYWH(100, 100, 200, 200);

  skity::Rect rect2 = skity::Rect::MakeXYWH(150, 150, 100, 100);

  clip_paint.setColor(skity::Color_BLACK);

  canvas->drawRect(rect1, clip_paint);

  canvas->save();

  canvas->clipRect(rect1);

  canvas->drawLine(100, 170, 400, 200, stroke_paint);

  canvas->rotate(degree, 170.f, 170.f);

  clip_paint.setColor(skity::Color_BLUE);

  canvas->drawRect(rect2, clip_paint);

  canvas->clipRect(rect2);

  canvas->rotate(-degree, 170.f, 170.f);

  canvas->drawLine(100, 180, 400, 230, stroke_paint);

  canvas->restore();
}