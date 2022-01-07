#include "gl/gl_app.hpp"

void draw_clip_demo(skity::Canvas* canvas);

class GLClipExample : public example::GLApp {
 public:
  GLClipExample()
      : example::GLApp(800, 800, "GL Clip Example", {1.f, 1.f, 1.f, 1.f}) {}
  ~GLClipExample() override = default;

 protected:
  void OnUpdate(float time) override {
    draw_clip_demo(GetCanvas());

    GetCanvas()->flush();
  }
};

int main(int argc, const char** argv) {
  GLClipExample app;
  app.Run();

  return 0;
}