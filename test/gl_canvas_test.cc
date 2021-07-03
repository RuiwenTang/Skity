
#include <skity/render/canvas.hpp>

#include "test/common/test_common.hpp"

class GLCanvasDemo : public test::TestApp {
 public:
  GLCanvasDemo() : TestApp() {}
  ~GLCanvasDemo() override = default;

 protected:
  void OnInit() override {
    canvas_ = skity::Canvas::MakeGLCanvas(0, 0, 800, 600);
    glClearColor(0.3, 0.4, 0.5, 1.0);
  }

  void OnDraw() override {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(10.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.SetStrokeColor(1.f, 0.f, 0.f, 1.f);
    paint.SetFillColor(1.f, 1.f, 0.f, 1.f);

    skity::Path path;
    path.moveTo(350, 50);
    path.cubicTo(556, 64, 310, 192, 550, 250);

    canvas_->drawPath(path, paint);

    canvas_->flush();
  }

 private:
  std::unique_ptr<skity::Canvas> canvas_;
};

int main(int argc, const char** argv) {
  GLCanvasDemo demo;
  demo.Start();
  return 0;
}