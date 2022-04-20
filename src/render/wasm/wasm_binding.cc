#include <emscripten/bind.h>

#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

using namespace emscripten;

// canvas binding

EMSCRIPTEN_BINDINGS(skity) {
  class_<skity::Rect>("Rect")
      .constructor()
      .constructor<float, float, float, float>()
      .class_function("MakeXYWH", &skity::Rect::MakeXYWH)
      .class_function("MakeWH", &skity::Rect::MakeWH)
      .function("setLTRB", &skity::Rect::setLTRB)
      .function("offset", &skity::Rect::offset)
      .property("left", &skity::Rect::left)
      .property("right", &skity::Rect::right)
      .property("top", &skity::Rect::top)
      .property("bottom", &skity::Rect::bottom);
    
  class_<skity::RRect>("RRect")
  .constructor()
  .function("setRect", &skity::RRect::setRect)
  .function("setRectXY", &skity::RRect::setRectXY)
  .function("setOval", &skity::RRect::setOval)
  .function("offset", &skity::RRect::offset);

  enum_<skity::Paint::Style>("Style")
      .value("Fill", skity::Paint::kFill_Style)
      .value("Stroke", skity::Paint::kStroke_Style);

  enum_<skity::Paint::Cap>("LineCap")
      .value("Round", skity::Paint::kRound_Cap)
      .value("Butt", skity::Paint::kButt_Cap)
      .value("Square", skity::Paint::kSquare_Cap);

  enum_<skity::Paint::Join>("LineJoin")
      .value("Round", skity::Paint::kRound_Join)
      .value("Mitter", skity::Paint::kMiter_Join)
      .value("Miter", skity::Paint::kMiter_Join);

  function("ColorSetARGB", &skity::ColorSetARGB);

  class_<skity::Path>("Path")
  .constructor()
  .function("moveTo", select_overload<skity::Path&(float, float)>(&skity::Path::moveTo))
  .function("lineTo", select_overload<skity::Path&(float, float)>(&skity::Path::lineTo))
  .function("quadTo", select_overload<skity::Path&(float, float, float, float)>(&skity::Path::quadTo))
  .function("close", &skity::Path::close);
  ;

  class_<skity::Paint>("Paint")
      .constructor()
      .function("setStyle", &skity::Paint::setStyle)
      .function("setStrokeWidth", &skity::Paint::setStrokeWidth)
      .function("setStrokeJoin", &skity::Paint::setStrokeJoin)
      .function("setStrokeCap", &skity::Paint::setStrokeCap)
      .function("setColor", &skity::Paint::setColor);

  class_<skity::Canvas>("Canvas")
      .class_function("Make", &skity::Canvas::MakeWebGLCanvas)
      .function("drawRect", &skity::Canvas::drawRect)
      .function("drawPath", &skity::Canvas::drawPath)
      .function("drawRRect", &skity::Canvas::drawRRect)
      .function("drawRoundRect", &skity::Canvas::drawRoundRect)
      .function("drawCircle", &skity::Canvas::drawCircle)
      .function("flush", &skity::Canvas::flush);
}