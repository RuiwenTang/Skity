#include <skity/gpu/gpu_context.hpp>
#include <skity/skity.hpp>

#include "test/common/test_common.hpp"

class FillRuleTest : public test::TestApp {
 public:
  FillRuleTest() = default;
  ~FillRuleTest() override = default;

  void OnInit() override {
    glClearColor(1.f, 1.f, 1.f, 1.f);
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
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    skity::Paint paint;
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setColor(skity::ColorSetA(skity::Color_RED, 64));

    skity::Path path;
    path.moveTo(100, 10);
    path.lineTo(40, 180);
    path.lineTo(190, 60);
    path.lineTo(10, 60);
    path.lineTo(160, 180);
    path.close();

    canvas_->drawPath(path, paint);

    canvas_->save();

    canvas_->translate(200, 0);

    path.setFillType(skity::Path::PathFillType::kEvenOdd);
    canvas_->drawPath(path, paint);

    canvas_->restore();

    canvas_->flush();
  }

 private:
  std::unique_ptr<skity::Canvas> canvas_ = {};
};

int main(int argc, const char** argv) {
  FillRuleTest app{};
  app.Start();
  return 0;
}
