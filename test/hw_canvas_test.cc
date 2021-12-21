#include <skity/codec/codec.hpp>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/effect/shader.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

#include "test/common/test_common.hpp"
#include "test_config.hpp"

class HWCanvasTest : public test::TestApp {
 public:
  HWCanvasTest() : TestApp(){};
  ~HWCanvasTest() override = default;

 protected:
  void OnInit() override {
    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glClearStencil(0x0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0x00);
    glStencilMask(0xFF);

    skity::GPUContext ctx{skity::GPUBackendType::kOpenGL,
                          (void*)glfwGetProcAddress};

    canvas_ =
        skity::Canvas::MakeHardwareAccelationCanvas(800, 600, Density(), &ctx);

    canvas_->setDefaultTypeface(
        skity::Typeface::MakeFromFile(TEST_BUILD_IN_FONT));

    auto skity_data = skity::Data::MakeFromFileName(TEST_BUILD_IN_IMAGE);

    if (skity_data) {
      auto codec = skity::Codec::MakeFromData(skity_data);

      if (codec) {
        codec->SetData(skity_data);
        pixmap_ = codec->Decode();
      }
    }
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setColor(skity::Color_RED);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.setStrokeWidth(20.f);

    {
      skity::Vec4 colors[] = {
          skity::Vec4{0.f, 1.f, 1.f, 1.f},
          skity::Vec4{0.f, 0.f, 1.f, 1.f},
          skity::Vec4{1.f, 0.f, 0.f, 1.f},
      };

      std::vector<skity::Point> pts = {
          skity::Point{100.f, 100.f, 0.f, 1.f},
          skity::Point{400.f, 200.f, 0.f, 1.f},
      };

      auto lgs = skity::Shader::MakeLinear(pts.data(), colors, nullptr, 3);
      paint.setShader(lgs);
    }

    canvas_->drawLine(100, 100, 400, 200, paint);

    paint.setShader(nullptr);

    skity::Path path;
    path.moveTo(100, 50);
    path.lineTo(300, 60);
    path.lineTo(150, 120);
    path.close();

    paint.setColor(skity::ColorSetARGB(128, 0, 0x9D, 0x58));
    canvas_->drawPath(path, paint);

    canvas_->drawRect(skity::Rect::MakeXYWH(300, 300, 50, 50), paint);

    paint.setStyle(skity::Paint::kFill_Style);
    paint.setColor(skity::ColorSetARGB(255, 0, 0x9D, 0x58));

    canvas_->save();

    canvas_->rotate(30, 225, 225);

    {
      if (pixmap_) {
        auto shader = skity::Shader::MakeShader(pixmap_);
        paint.setShader(shader);
      }
    }

    canvas_->drawRect(skity::Rect::MakeXYWH(200, 200, 50, 50), paint);
    paint.setShader(nullptr);

    canvas_->restore();

    paint.setColor(skity::ColorSetARGB(128, 255, 0x9D, 0x58));

    {
      skity::Point center{400, 400, 0, 1.f};
      skity::Vec4 radialColors[] = {skity::Vec4{1.f, 1.f, 1.f, 1.f},
                                    skity::Vec4{0.f, 0.f, 0.f, 1.f}};
      float pts[] = {0.f, 1.f};
      auto rgs =
          skity::Shader::MakeRadial(center, 120.f, radialColors, nullptr, 2);
      paint.setShader(rgs);
    }

    canvas_->drawCircle(400, 400, 100, paint);

    paint.setShader(nullptr);

    paint.setStyle(skity::Paint::kFill_Style);
    paint.setStrokeWidth(10.f);

    canvas_->drawRoundRect(skity::Rect::MakeXYWH(500, 200, 100, 100), 30.f,
                           30.f, paint);

    skity::Path clip_path;
    clip_path.addRect(skity::Rect::MakeXYWH(520, 400, 100, 80));

    paint.setColor(skity::Color_BLUE);
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(2.f);

    degree_ += 0.01f;
    if (degree_ >= 360.f) {
      degree_ = 0.f;
    }

    canvas_->save();
    canvas_->rotate(degree_, 570, 440);
    canvas_->drawPath(clip_path, paint);
    canvas_->clipPath(clip_path);
    canvas_->rotate(-degree_, 570, 440);
    paint.setStrokeWidth(10.f);
    paint.setColor(skity::ColorSetARGB(64, 255, 0, 0));
    {
      skity::Path temp_path;
      temp_path.moveTo(500, 380);
      temp_path.lineTo(600, 450);
      temp_path.lineTo(600, 400);
      canvas_->drawPath(temp_path, paint);
    }
    canvas_->restore();

    paint.setColor(skity::Color_LTGRAY);
    canvas_->drawLine(500, 420, 600, 470, paint);

    paint.setStyle(skity::Paint::kFill_Style);
    paint.setTextSize(20.f);
    canvas_->drawSimpleText2("12ATx", 500, 300, paint);

    paint.setColor(skity::Color_GREEN);
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(2.f);
    canvas_->drawLine(500, 300, 600, 300, paint);

    canvas_->flush();
  }

 private:
  std::unique_ptr<skity::Canvas> canvas_ = {};
  std::shared_ptr<skity::Pixmap> pixmap_ = {};
  float degree_ = 0.f;
};

int main(int argc, const char** argv) {
  HWCanvasTest app{};
  app.Start();
  return 0;
}