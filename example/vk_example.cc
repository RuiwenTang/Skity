
#include "vk/vk_app.hpp"

void draw_canvas(skity::Canvas* canvas);

class VKExample : public example::VkApp {
 public:
  VKExample()
      : example::VkApp(1000, 800, "Vulkan Example", {1.f, 1.f, 1.f, 1.f}) {}
  ~VKExample() override = default;

 protected:
  void OnUpdate(float elapsed_time) override {
    draw_canvas(GetCanvas());

    GetCanvas()->flush();
  }
};

int main(int argc, const char** argv) {
  VKExample app;
  app.Run();

  return 0;
}