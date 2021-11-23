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
    paint.setStrokeWidth(20.f);

    canvas_->drawLine(100, 100, 400, 200, paint);

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