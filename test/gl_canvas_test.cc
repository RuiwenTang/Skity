
#include <skity/render/canvas.hpp>

#include "test/common/test_common.hpp"

class GLCanvasDemo : public test::TestApp {
 public:
  GLCanvasDemo() : TestApp() {}
  ~GLCanvasDemo() override = default;

 protected:
  void OnInit() override {
    canvas_ =
        skity::Canvas::MakeGLCanvas(0, 0, 800, 600, (void*)glfwGetProcAddress);
    // FIXME: when implement Canvas::clearColor(); remove this code
    glClearColor(0.3, 0.4, 0.5, 1.0);
    glClearStencil(0x00);
    glStencilMask(0xff);
  }

  void OnDraw() override {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kStrokeAndFill_Style);
    paint.setStrokeWidth(8.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.SetStrokeColor(1.f, 0.f, 0.f, 1.f);
    paint.SetFillColor(1.f, 1.f, 0.f, 1.f);

    skity::Path path;
    path.moveTo(350, 50);
    path.cubicTo(556, 64, 310, 192, 550, 250);
    path.close();

    canvas_->save();
    canvas_->translate(100, 100);
    canvas_->drawPath(path, paint);
    canvas_->restore();

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
    paint2.setStyle(skity::Paint::kStrokeAndFill_Style);

    canvas_->drawPath(path2, paint2);

    skity::Path path3;
    path3.addCircle(200, 150, 100);

    canvas_->save();
    canvas_->clipPath(path2);
    canvas_->drawPath(path3, paint);
    canvas_->restore();

    canvas_->drawDebugLine(true);
    paint.setStyle(skity::Paint::kFill_Style);
    paint.SetFillColor(1.f, 0.f, 0.f, 1.f);
    canvas_->drawSimpleText("Hello World", 100, 450, paint);
    canvas_->drawDebugLine(false);

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