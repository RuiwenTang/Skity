#include <skity/svg/svg_dom.hpp>

#include "gl/gl_app.hpp"

std::unique_ptr<skity::SVGDom> init_simple_svg();

class GLSVGApp : public example::GLApp {
 public:
  GLSVGApp()
      : example::GLApp(1000, 1000, "OpenGL SVG Example", {1.f, 1.f, 1.f, 1.f}) {
  }

  ~GLSVGApp() override = default;

 protected:
  void OnStart() override { svg_ = init_simple_svg(); }

  void OnUpdate(float) override {
    GetCanvas()->save();
    GetCanvas()->translate(50, 50);

    svg_->Render(GetCanvas());

    GetCanvas()->restore();

    GetCanvas()->flush();
  }

 private:
  std::unique_ptr<skity::SVGDom> svg_ = {};
};

int main(int argc, const char** argv) {
  GLSVGApp app;
  app.Run();

  return 0;
}