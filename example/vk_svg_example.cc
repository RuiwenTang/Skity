#include <skity/svg/svg_dom.hpp>

#include "vk/vk_app.hpp"

std::unique_ptr<skity::SVGDom> init_simple_svg();

class VkSVGApp : public example::VkApp {
 public:
  VkSVGApp()
      : example::VkApp(1000, 1000, "Vulkan SVG Example", {1.f, 1.f, 1.f, 1.f}) {
  }
  ~VkSVGApp() override = default;

 protected:
  void OnStart() override {
    VkApp::OnStart();

    svg_ = init_simple_svg();
  }

  void OnUpdate(float elapsed_time) override {
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
  VkSVGApp app;
  app.Run();

  return 0;
}