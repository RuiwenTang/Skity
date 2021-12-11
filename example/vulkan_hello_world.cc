#include <cstring>
#include <example_config.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <skity/skity.hpp>
#include <vector>

#include "vk/vk_app.hpp"

class HelloVulkanApp : public example::VkApp, public skity::GPUVkContext {
 public:
  HelloVulkanApp()
      : VkApp(800, 800, "Hello Vulkan"),
        skity::GPUVkContext((void*)vkGetDeviceProcAddr) {}
  ~HelloVulkanApp() override = default;

  VkInstance GetInstance() override { return example::VkApp::Instance(); }
  VkPhysicalDevice GetPhysicalDevice() override {
    return example::VkApp::PhysicalDevice();
  }
  VkDevice GetDevice() override { return example::VkApp::Device(); }
  VkExtent2D GetFrameExtent() override { return example::VkApp::FrameExtent(); }
  VkCommandBuffer GetCurrentCMD() override {
    return example::VkApp::CurrentCMDBuffer();
  }
  VkRenderPass GetRenderPass() override { return example::VkApp::RenderPass(); }
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr() override {
    return &vkGetInstanceProcAddr;
  }
 protected:
  void OnStart() override {
    canvas_ = skity::Canvas::MakeHardwareAccelationCanvas(
        800, 800, ScreenDensity(), this);
  }
  void OnUpdate(float elapsed_time) override {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setColor(skity::Color_RED);

    canvas_->drawRect(skity::Rect::MakeXYWH(100, 100, 200, 200), paint);

    canvas_->flush();
  }
  void OnDestroy() override { canvas_ = nullptr; }

 private:
  std::unique_ptr<skity::Canvas> canvas_ = {};
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
