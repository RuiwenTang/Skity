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
        skity::Canvas::MakeGLCanvas(0, 0, 200, 150, (void*)glfwGetProcAddress);

    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glClearStencil(0x0);
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

    skity::Path path;

    path.moveTo(50, 20);
    path.lineTo(100, 60);
    path.lineTo(70, 140);
    path.close();

    canvas_->drawPath(path, paint);

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