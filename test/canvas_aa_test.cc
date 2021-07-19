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
    paint.setStyle(skity::Paint::kStrokeAndFill_Style);
    paint.setStrokeWidth(8.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.SetStrokeColor(1.f, 0.f, 0.f, 1.f);
    paint.SetFillColor(1.f, 1.f, 0.f, 1.f);

    paint.setAntiAlias(true);

    skity::Path path;

    path.moveTo(50, 20);
    path.lineTo(100, 60);
    path.lineTo(70, 140);
    path.close();

    path.moveTo(350, 50);
    path.cubicTo(556, 64, 310, 192, 550, 250);
    path.close();

    canvas_->drawPath(path, paint);

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

    skity::Paint paint2;
    paint2.setStrokeWidth(3.f);
    paint2.setStrokeJoin(skity::Paint::kRound_Join);
    paint2.setStrokeCap(skity::Paint::kRound_Cap);
    paint2.SetStrokeColor(0, 0, 1, 1);
    paint2.SetFillColor(150.f / 255.f, 150.f / 255.f, 1.f, 1.f);
    paint2.setStyle(skity::Paint::kFill_Style);
    paint2.setAntiAlias(true);

    canvas_->drawPath(path2, paint2);

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