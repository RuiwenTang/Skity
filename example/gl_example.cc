
#include "gl/gl_app.hpp"

void draw_canvas(skity::Canvas* canvas);

class ExampleApp : public example::GLApp {
 public:
  ExampleApp()
      : example::GLApp(1000, 800, "GL Example", {1.f, 1.f, 1.f, 1.f}) {}

  ~ExampleApp() override = default;

 protected:
  void OnUpdate(float time) override {
    draw_canvas(GetCanvas());
    GetCanvas()->flush();
  }
};

int main(int argc, const char** argv) {
  ExampleApp app;

  app.Run();
  return 0;
}