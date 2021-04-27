#include "src/render/stroke.hpp"

int main(int argc, const char** argv)
{
  skity::Path src;
  skity::Path dst;
  skity::Paint paint;

  paint.setStrokeWidth(4.0);
  paint.setStrokeCap(skity::Paint::kRound_Cap);
  paint.setStrokeJoin(skity::Paint::kMiter_Join);

  src.moveTo(10, 10);
  src.lineTo(20, 20);
  src.lineTo(30, 10);

  skity::Stroke stroke(paint);

  stroke.strokePath(src, &dst);

  dst.dump();
  return 0;
}