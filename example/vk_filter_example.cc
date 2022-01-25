#include "vk/vk_app.hpp"

void draw_filter(skity::Canvas* canvas);

class VKFilterExample : public example::VkApp {
 public:
  VKFilterExample()
      : example::VkApp(800, 600, "Vulkan Filter Example",
                       {0.3f, 0.4f, 0.5f, 1.f}) {}

  ~VKFilterExample() override = default;

 protected:
  void OnUpdate(float time) override {
    draw_filter(GetCanvas());

    GetCanvas()->flush();
  }
};

int main(int argc, const char** argv) {
  VKFilterExample app;
  app.Run();

  return 0;
}