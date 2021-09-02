#include <glad/glad.h>
// Fixme glad header must include first
#include <GLFW/glfw3.h>

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

#include "test/common/test_common.hpp"

class CanvasAATest : public test::TestApp {
 public:
  CanvasAATest() : TestApp() {}
  ~CanvasAATest() override = default;

 protected:
  void OnInit() override {
    canvas_ =
        skity::Canvas::MakeGLCanvas(0, 0, 800, 600, (void*)glfwGetProcAddress);

    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glClearStencil(0x0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  }

  void OnWindowSizeUpdate(uint32_t width, uint32_t height) override {
    canvas_->updateViewport(width, height);
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kStrokeAndFill_Style);
    paint.setStrokeWidth(8.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.SetStrokeColor(0.3f, 0.f, 0.f, 0.3f);
    paint.SetFillColor(0.3f, 0.3f, 0.f, 0.3f);

    skity::Path path;

    path.moveTo(50, 50);
    path.cubicTo(256, 64, 10, 192, 250, 450);
    path.close();

//    canvas_->drawPath(path, paint);
//
//    canvas_->save();
//    canvas_->translate(100, 0);
//    paint.setAntiAlias(true);
//    canvas_->drawPath(path, paint);
//    canvas_->restore();

    skity::Paint font_paint;
//    font_paint.setAntiAlias(true);
    font_paint.setStyle(skity::Paint::kFill_Style);
    font_paint.SetFillColor(.3f, 0.f, 0.f, .3f);
    font_paint.setTextSize(50.f);

    canvas_->drawSimpleText2("Hello World no AA", 250, 50, font_paint);
    font_paint.setAntiAlias(true);
    font_paint.setTextSize(30.f);
    canvas_->drawSimpleText("Hello World with AA", 250, 120, font_paint);

//    skity::Rect rect{400, 400, 600, 500};

//    skity::Path mix_path;
//    mix_path.addRRect(skity::RRect::MakeRectXY(rect, 10, 10));

//    canvas_->drawPath(mix_path, paint);
    canvas_->flush();
  }

 private:
  std::unique_ptr<skity::Canvas> canvas_;
};

int main(int argc, const char** argv) {
  CanvasAATest aa_test;
  aa_test.Start();
  return 0;
}