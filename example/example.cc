
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/skity.hpp>
#include <string>

// same as https://fiddle.skia.org/c/@shapes
static void draw_basic_example(skity::Canvas* canvas) {
  skity::Paint paint;
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setAntiAlias(true);
  paint.setStrokeWidth(4.f);
  paint.SetFillColor(0x42 / 255.f, 0x85 / 255.f, 0xF4 / 255.f, 1.f);

  skity::Rect rect = skity::Rect::MakeXYWH(10, 10, 100, 160);
  canvas->drawRect(rect, paint);

  skity::RRect oval;
  oval.setOval(rect);
  oval.offset(40, 80);
  paint.SetFillColor(0xDB / 255.f, 0x44 / 255.f, 0x37 / 255.f, 1.f);
  canvas->drawRRect(oval, paint);

  paint.SetFillColor(0x0F / 255.f, 0x9D / 255.f, 0x58 / 255.f, 1.f);
  canvas->drawCircle(180, 50, 25, paint);

  rect.offset(80, 50);
  paint.SetStrokeColor(0xF4 / 255.f, 0xB4 / 255.f, 0x0, 1.f);
  paint.setStyle(skity::Paint::kStroke_Style);
  canvas->drawRoundRect(rect, 10, 10, paint);
}

// same as https://fiddle.skia.org/c/@discrete_path
static void draw_path_effect_example(skity::Canvas* canvas) {
  const float R = 115.2f, C = 128.f;
  skity::Path path;
  path.moveTo(C + R, C);
  for (int32_t i = 1; i < 8; i++) {
    float a = 2.6927937f * i;
    path.lineTo(C + R * std::cos(a), C + R * std::sin(a));
  }

  skity::Paint paint;
  paint.setPathEffect(skity::PathEffect::MakeDiscretePathEffect(10.f, 4.f));
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setStrokeWidth(2.f);
  paint.setAntiAlias(true);
  paint.SetStrokeColor(0x42 / 255.f, 0x85 / 255.f, 0xF4 / 255.f, 1.f);
  canvas->drawPath(path, paint);
}

static void draw_dash_start_example(skity::Canvas* canvas) {
  skity::Path path;
  path.moveTo(199, 34);
  path.lineTo(253, 143);
  path.lineTo(374, 160);
  path.lineTo(287, 244);
  path.lineTo(307, 365);
  path.lineTo(199, 309);
  path.lineTo(97, 365);
  path.lineTo(112, 245);
  path.lineTo(26, 161);
  path.lineTo(146, 143);
  path.close();

  skity::Paint paint;
  paint.setStrokeWidth(3.f);
  paint.setStrokeJoin(skity::Paint::kRound_Join);
  paint.setStrokeCap(skity::Paint::kRound_Cap);
  paint.SetStrokeColor(0, 0, 1, 1);
  paint.SetFillColor(150.f / 255.f, 150.f / 255.f, 1.f, 1.f);
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kStrokeAndFill_Style);
  float pattern[2] = {10.f, 10.f};
  paint.setPathEffect(skity::PathEffect::MakeDashPathEffect(pattern, 2, 0));

  canvas->drawPath(path, paint);
}

// same as https://fiddle.skia.org/c/844ab7d5e63876f6c889b33662ece8d5
void draw_linear_gradient_example(skity::Canvas* canvas) {
  skity::Paint p;
  p.setStyle(skity::Paint::kFill_Style);

  skity::Vec4 colors[] = {
      skity::Vec4{0.f, 1.f, 1.f, 0.f},
      skity::Vec4{0.f, 0.f, 1.f, 1.f},
      skity::Vec4{1.f, 0.f, 0.f, 1.f},
  };
  float positions[] = {0.f, 0.65f, 1.f};

  for (int i = 0; i < 4; i++) {
    float blockX = (i % 2) * 100.f;
    float blockY = (i / 2) * 100.f;

    std::vector<skity::Point> pts = {
        skity::Point{blockX, blockY, 0.f, 1.f},
        skity::Point{blockX + 50, blockY + 100, 0.f, 1.f},
    };

    skity::Matrix matrix = glm::identity<skity::Matrix>();
    int flag = 0;
    if (i % 2 == 1) {
      flag = 1;
    }
    if (i / 2 == 1) {
      matrix *= glm::translate(glm::identity<glm::mat4>(),
                               glm::vec3(blockX, blockY, 0));
      matrix *= glm::rotate(glm::identity<glm::mat4>(), glm::radians(45.f),
                            glm::vec3(0, 0, 1));
      matrix *= glm::translate(glm::identity<glm::mat4>(),
                               glm::vec3(-blockX, -blockY, 0));
    }
    auto lgs =
        skity::Shader::MakeLinear(pts.data(), colors, positions, 3, flag);
    lgs->SetLocalMatrix(matrix);
    p.setShader(lgs);
    auto r = skity::Rect::MakeLTRB(blockX, blockY, blockX + 100, blockY + 100);
    canvas->drawRect(r, p);
  }

  skity::Path circle;
  circle.addCircle(220, 350, 100);
  skity::Paint paint;
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setAntiAlias(true);
  skity::Point center{220, 350, 0, 1};
  skity::Vec4 radialColors[] = {skity::Vec4{1.f, 1.f, 1.f, 1.f},
                                skity::Vec4{0.f, 0.f, 0.f, 1.f}};
  float pts[] = {0.f, 1.f};
  auto rgs = skity::Shader::MakeRadial(center, 150.f, radialColors, nullptr, 2);
  paint.setShader(rgs);

  canvas->drawPath(circle, paint);
}

// same as https://fiddle.skia.org/c/@text_rendering
void draw_simple_text(skity::Canvas* canvas) {
  skity::Paint paint;

  paint.setTextSize(64.f);
  paint.setAntiAlias(true);
  paint.SetFillColor(0x42 / 255.f, 0x85 / 255.f, 0xF4 / 255.f, 1.f);
  paint.setStyle(skity::Paint::kFill_Style);

  skity::TextBlobBuilder builder;
  paint.setTypeface(canvas->getDefaultTypeface());
  auto blob = builder.buildTextBlob("Skity", paint);

  canvas->drawTextBlob(blob.get(), 20.f, 64.f, paint);

  paint.setStyle(skity::Paint::kStroke_Style);
  paint.SetStrokeColor(0xDB / 255.f, 0x44 / 255.f, 0x37 / 255.f, 1.f);
  paint.setStrokeWidth(2.f);

  canvas->drawTextBlob(blob.get(), 20.f, 144.f, paint);

  paint.SetFillColor(0x0F / 255.f, 0x9D / 255.f, 0x58 / 255.f, 1.f);
  paint.setStyle(skity::Paint::kFill_Style);

  canvas->save();

  skity::Vec4 colors[] = {
      skity::Vec4{0.f, 1.f, 1.f, 1.f},
      skity::Vec4{0.f, 0.f, 1.f, 1.f},
      skity::Vec4{1.f, 0.f, 0.f, 1.f},
  };

  std::vector<skity::Point> pts = {
      skity::Point{0.f, 0.f, 0.f, 1.f},
      skity::Point{200.f, 0.f, 0.f, 1.f},
  };

  auto lgs = skity::Shader::MakeLinear(pts.data(), colors, nullptr, 3);
  paint.setShader(lgs);

  canvas->drawTextBlob(blob.get(), 20.f, 224.f, paint);
  canvas->restore();
}

void draw_canvas(skity::Canvas* canvas) {
  draw_basic_example(canvas);

  canvas->save();
  canvas->translate(300, 0);
  draw_path_effect_example(canvas);
  canvas->restore();

  canvas->save();
  canvas->translate(0, 300);
  draw_dash_start_example(canvas);
  canvas->restore();

  canvas->save();
  canvas->translate(520, 0);
  draw_simple_text(canvas);
  canvas->restore();

  canvas->save();
  canvas->translate(400, 300);
  draw_linear_gradient_example(canvas);
  canvas->restore();
}
