#include <emscripten/bind.h>

#include <skity/effect/path_effect.hpp>
#include <skity/effect/shader.hpp>
#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/io/data.hpp>
#include <skity/render/canvas.hpp>
#include <skity/text/text_blob.hpp>
#include <skity/text/typeface.hpp>

using namespace emscripten;

// handle data convert
namespace skity {

std::shared_ptr<Data> MakeCopyWithString(std::string const& str) {
  if (str.empty()) {
    return nullptr;
  }

  return Data::MakeWithCopy(str.c_str(), str.length());
}

std::shared_ptr<Shader> MakeRadialShader(float cx, float cy, float radius,
                                         std::vector<uint32_t> const& colors) {
  if (colors.size() < 2) {
    return nullptr;
  }

  Point center{cx, cy, 0.f, 1.f};

  std::vector<Color4f> f_colors;

  for (auto c : colors) {
    f_colors.emplace_back(Color4fFromColor(c));
  }

  return Shader::MakeRadial(center, radius, f_colors.data(), nullptr,
                            f_colors.size());
}

std::shared_ptr<Shader> MakeLinearShader(float x1, float y1, float x2, float y2,
                                         std::vector<uint32_t> const& colors) {
  if (colors.size() < 2) {
    return nullptr;
  }

  std::array<skity::Point, 2> pts{skity::Point{x1, y1, 0.f, 1.f},
                                  skity::Point{x2, y2, 0.f, 1.f}};

  std::vector<Color4f> f_colors;

  for (auto c : colors) {
    f_colors.emplace_back(Color4fFromColor(c));
  }

  return Shader::MakeLinear(pts.data(), f_colors.data(), nullptr,
                            f_colors.size());
}

std::shared_ptr<Shader> MakeLinearShaderWithPos(
    float x1, float y1, float x2, float y2, std::vector<uint32_t> const& colors,
    std::vector<float> const& pos) {
  if (colors.size() < 2) {
    return nullptr;
  }

  if (colors.size() != pos.size()) {
    return nullptr;
  }

  std::array<skity::Point, 2> pts{skity::Point{x1, y1, 0.f, 1.f},
                                  skity::Point{x2, y2, 0.f, 1.f}};

  std::vector<Color4f> f_colors;

  for (auto c : colors) {
    f_colors.emplace_back(Color4fFromColor(c));
  }

  return Shader::MakeLinear(pts.data(), f_colors.data(), pos.data(),
                            f_colors.size());
}

std::shared_ptr<PathEffect> MakeDashEffect(std::vector<float> const& pattern) {
  return PathEffect::MakeDashPathEffect(pattern.data(), pattern.size(), 0);
}

glm::mat4 MatrixTranslate(glm::mat4 const& m, float x, float y) {
  return glm::translate(m, glm::vec3(x, y, 0.f));
}

glm::mat4 MatrixRotate(glm::mat4 const& m, float angle, float x, float y,
                       float z) {
  return glm::rotate(m, angle, glm::vec3(x, y, z));
}

glm::mat4 MatrixMultiply(glm::mat4 const& m1, glm::mat4 const& m2) {
  return m1 * m2;
}

}  // namespace skity

// canvas binding

EMSCRIPTEN_BINDINGS(skity) {
  register_vector<uint32_t>("VectorUint32");
  register_vector<float>("VectorFloat");

  class_<skity::Matrix>("Matrix")
      .constructor(&glm::identity<glm::mat4>)
      .class_function("Translate", &skity::MatrixTranslate)
      .class_function("Rotate", &skity::MatrixRotate)
      .class_function("Multiply", &skity::MatrixMultiply);

  class_<skity::Shader>("Shader")
      .smart_ptr<std::shared_ptr<skity::Shader>>("Shader")
      .function("setLocalMatrix", &skity::Shader::SetLocalMatrix)
      .class_function("MakeLinear", &skity::MakeLinearShader)
      .class_function("MakeLinearWithPos", &skity::MakeLinearShaderWithPos)
      .class_function("MakeRadial", &skity::MakeRadialShader);

  class_<skity::Data>("Data")
      .smart_ptr<std::shared_ptr<skity::Data>>("Data")
      .property("size", &skity::Data::Size)
      .class_function("MakeWithCopy", &skity::MakeCopyWithString);

  class_<skity::PathEffect>("PathEffect")
      .smart_ptr<std::shared_ptr<skity::PathEffect>>("PathEffect")
      .class_function("MakeDiscretePathEffect",
                      &skity::PathEffect::MakeDiscretePathEffect)
      .class_function("MakeDashEffect", &skity::MakeDashEffect);

  class_<skity::TextBlob>("TextBlob")
      .smart_ptr<std::shared_ptr<skity::TextBlob>>("TextBlob");

  class_<skity::TextBlobBuilder>("TextBlobBuilder")
      .constructor()
      .function("buildTextBlob",
                select_overload<std::shared_ptr<skity::TextBlob>(
                    std::string const&, skity::Paint const&)>(
                    &skity::TextBlobBuilder::buildTextBlob));

  class_<skity::Rect>("Rect")
      .constructor()
      .constructor<float, float, float, float>()
      .class_function("MakeXYWH", &skity::Rect::MakeXYWH)
      .class_function("MakeWH", &skity::Rect::MakeWH)
      .class_function("MakeLTRB", &skity::Rect::MakeLTRB)
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

  enum_<skity::Path::Direction>("PathDirection")
      .value("CW", skity::Path::Direction::kCW)
      .value("CCW", skity::Path::Direction::kCCW);

  enum_<skity::Path::PathFillType>("PathFillType")
      .value("Winding", skity::Path::PathFillType::kWinding)
      .value("EvenOdd", skity::Path::PathFillType::kEvenOdd);

  function("ColorSetARGB", &skity::ColorSetARGB);

  class_<skity::Path>("Path")
      .constructor()
      .function("setFillType", &skity::Path::setFillType)
      .function("moveTo", select_overload<skity::Path&(float, float)>(
                              &skity::Path::moveTo))
      .function("lineTo", select_overload<skity::Path&(float, float)>(
                              &skity::Path::lineTo))
      .function("quadTo",
                select_overload<skity::Path&(float, float, float, float)>(
                    &skity::Path::quadTo))
      .function("addCircle", &skity::Path::addCircle)
      .function("close", &skity::Path::close);

  class_<skity::Typeface>("Typeface")
      .smart_ptr<std::shared_ptr<skity::Typeface>>("Typeface")
      .class_function("MakeFromData", &skity::Typeface::MakeFromData);

  class_<skity::Paint>("Paint")
      .constructor()
      .function("setStyle", &skity::Paint::setStyle)
      .function("setStrokeWidth", &skity::Paint::setStrokeWidth)
      .function("setStrokeJoin", &skity::Paint::setStrokeJoin)
      .function("setStrokeCap", &skity::Paint::setStrokeCap)
      .function("setColor", &skity::Paint::setColor)
      .function("setTypeface", &skity::Paint::setTypeface)
      .function("setTextSize", &skity::Paint::setTextSize)
      .function("setShader", &skity::Paint::setShader)
      .function("setPathEffect", &skity::Paint::setPathEffect);

  class_<skity::Canvas>("Canvas")
      .class_function("Make", &skity::Canvas::MakeWebGLCanvas)
      .function("save", &skity::Canvas::save)
      .function("translate", &skity::Canvas::translate)
      .function("restore", &skity::Canvas::restore)
      .function("drawRect", &skity::Canvas::drawRect)
      .function("drawPath", &skity::Canvas::drawPath)
      .function("drawRRect", &skity::Canvas::drawRRect)
      .function("drawRoundRect", &skity::Canvas::drawRoundRect)
      .function("drawCircle", &skity::Canvas::drawCircle)
      .function("drawTextBlob",
                select_overload<void(std::shared_ptr<skity::TextBlob> const&,
                                     float, float, skity::Paint const&)>(
                    &skity::Canvas::drawTextBlob))
      .function("flush", &skity::Canvas::flush);
}