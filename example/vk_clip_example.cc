#include "vk/vk_app.hpp"

void draw_clip_demo(skity::Canvas* canvas);

class VKClipExample : public example::VkApp {
 public:
  VKClipExample()
      : example::VkApp(800, 800, "Vulkan Clip Example", {1.f, 1.f, 1.f, 1.f}) {}
  ~VKClipExample() override = default;

 protected:
  void OnUpdate(float elapsed_time) override {
    draw_clip_demo(GetCanvas());

    GetCanvas()->flush();
  }
};

int main(int argc, const char** argv) {
  VKClipExample app;
  app.Run();

  return 0;
}