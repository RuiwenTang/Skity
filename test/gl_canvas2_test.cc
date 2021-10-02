
#include <skity/effect/shader.hpp>
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
    paint.setStrokeWidth(10.f);
    paint.setStrokeCap(skity::Paint::kSquare_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);

    skity::Path path1;

    path1.moveTo(100, 100);
    path1.lineTo(300, 120);
    path1.lineTo(400, 300);
    path1.close();

    canvas_->save();

    canvas_->translate(100, 0);
    paint.setColor(skity::Color_DKGRAY);
    paint.setStyle(skity::Paint::kFill_Style);
    canvas_->drawPath(path1, paint);

    canvas_->restore();

    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setColor(skity::Color_WHITE);

    {
      skity::Vec4 colors[] = {
          skity::Vec4{0.f, 1.f, 1.f, 0.f},
          skity::Vec4{0.f, 0.f, 1.f, 1.f},
          skity::Vec4{1.f, 0.f, 0.f, 1.f},
      };

      float positions[] = {0.f, 0.65f, 1.f};

      std::vector<skity::Point> pts = {
          skity::Point{100, 100, 0.f, 1.f},
          skity::Point{420, 320, 0.f, 1.f},
      };

      auto lgs = skity::Shader::MakeLinear(pts.data(), colors, positions, 3, 0);

      paint.setShader(lgs);
    }

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