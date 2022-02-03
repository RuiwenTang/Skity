#include <skity/skity.hpp>

#include "glad/glad.h"

void draw_filter(skity::Canvas* canvas) {
  auto filter = skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 10.f);

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

  paint.setStyle(skity::Paint::kFill_Style);
  paint.setMaskFilter(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 20.f));

  canvas->save();
  canvas->translate(50.f, 0.f);
  canvas->drawCircle(300, 100, 50, paint);
  canvas->restore();

  canvas->save();
  canvas->translate(200.f, 0.f);

  paint.setMaskFilter(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kOuter, 20.f));
  canvas->drawCircle(300.f, 100.f, 50.f, paint);

  canvas->restore();

  canvas->save();

  canvas->translate(350, 0.f);

  paint.setMaskFilter(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kInner, 20.f));

  canvas->drawCircle(300.f, 100.f, 50.f, paint);
  canvas->restore();

  skity::Path path2;
  path2.moveTo(199, 34);
  path2.lineTo(253, 143);
  path2.lineTo(374, 160);
  path2.lineTo(287, 244);
  path2.lineTo(307, 365);
  path2.lineTo(199, 309);
  path2.lineTo(97, 365);
  path2.lineTo(112, 245);
  path2.lineTo(26, 161);
  path2.lineTo(146, 143);
  path2.close();

  paint.setMaskFilter(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 10.f));

  canvas->save();
  canvas->translate(300, 200);
  canvas->drawPath(path2, paint);
  canvas->restore();

  canvas->save();
  canvas->translate(0, 300);
  paint.setTextSize(40.f);
  paint.setMaskFilter(nullptr);
  paint.setStyle(skity::Paint::kFill_Style);

  canvas->drawSimpleText2("Hello World!", 10, 80, paint);
  paint.setMaskFilter(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, 3.f));
  canvas->drawSimpleText2("Hello World!", 10, 30, paint);
  canvas->restore();
}