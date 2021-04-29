#include "src/render/path_raster.hpp"

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

#include "src/render/stroke.hpp"

int main(int argc, const char** argv)
{
  skity::Path path;
  path.moveTo(10, 10);
  path.lineTo(20, 20);
  path.lineTo(30, 10);

  skity::Paint paint;
  paint.setStrokeWidth(4.f);
  paint.setStrokeCap(skity::Paint::kRound_Cap);
  paint.setStrokeJoin(skity::Paint::kMiter_Join);

  skity::Path dst;
  skity::Stroke stroke(paint);
  stroke.strokePath(path, &dst);

  skity::PathRaster raster;

  auto result = raster.rasterPath(dst);
  result->dump();

  return 0;
}
