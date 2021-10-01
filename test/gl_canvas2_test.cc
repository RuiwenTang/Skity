
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

#include "test/common/test_common.hpp"

class GLCanvas2Test : public test::TestApp {
 public:
  GLCanvas2Test() : test::TestApp() {}
  ~GLCanvas2Test() override = default;

 protected:
  void OnInit() override {
    canvas_ =
        skity::Canvas::MakeGLCanvas2(0, 0, 800, 600, (void*)glfwGetProcAddress);

    glClearColor(0.3, 0.4, 0.5, 1.0);
    glClearStencil(0x00);
    glStencilMask(0xff);

    // Blend is need for anti-alias
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  }
  void OnDraw() override {
    glStencilMask(0xFF);
    glColorMask(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setStrokeWidth(10.f);
    paint.setStrokeCap(skity::Paint::kSquare_Cap);
    paint.setStrokeJoin(skity::Paint::kMiter_Join);
    paint.setColor(skity::Color_WHITE);

    skity::Path path1;

    path1.moveTo(100, 100);
    path1.lineTo(300, 120);
    path1.lineTo(400, 300);
    path1.close();

    canvas_->drawPath(path1, paint);

    canvas_->flush();
  }

 private:
  std::unique_ptr<skity::Canvas> canvas_;
};

int main(int argc, const char** argv) {
  GLCanvas2Test app;
  app.Start();
  return 0;
}