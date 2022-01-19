#include "gl/gl_app.hpp"

void draw_filter(skity::Canvas* canvas);

class FilterExample : public example::GLApp {
 public:
  FilterExample()
      : example::GLApp(800, 600, "GL Filter Example", {1.f, 1.f, 1.f, 1.f}) {}

  ~FilterExample() override = default;

 protected:
  void OnUpdate(float time) override {
    draw_filter(GetCanvas());

    GetCanvas()->flush();
  }
};

int main(int argc, const char** argv) {
  FilterExample app;

  app.Run();

  return 0;
}
