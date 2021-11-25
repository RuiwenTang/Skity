#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

#include "test/common/test_common.hpp"

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

    canvas_ = skity::Canvas::MakeHardwareAccelationCanvas(
        800, 600, (void*)glfwGetProcAddress);
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setColor(skity::Color_RED);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.setStrokeWidth(20.f);

    canvas_->drawLine(100, 100, 400, 200, paint);

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

    canvas_->drawRect(skity::Rect::MakeXYWH(200, 200, 50, 50), paint);

    canvas_->restore();

    paint.setColor(skity::ColorSetARGB(128, 255, 0x9D, 0x58));
    canvas_->drawCircle(400, 400, 100, paint);

    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(10.f);

    canvas_->drawRoundRect(skity::Rect::MakeXYWH(500, 200, 100, 100), 30.f,
                           30.f, paint);

    canvas_->flush();
  }

 private:
  std::unique_ptr<skity::Canvas> canvas_ = {};
};

int main(int argc, const char** argv) {
  HWCanvasTest app{};
  app.Start();
  return 0;
}