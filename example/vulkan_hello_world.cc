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

  uint32_t GetSwapchainBufferCount() override {
    return example::VkApp::SwapchinImageCount();
  }

  uint32_t GetCurrentBufferIndex() override {
    return example::VkApp::CurrentFrameIndex();
  }

 protected:
  void OnStart() override {
    canvas_ = skity::Canvas::MakeHardwareAccelationCanvas(
        800, 800, ScreenDensity(), this);
  }
  void OnUpdate(float elapsed_time) override {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(4.f);
    paint.SetFillColor(0x42 / 255.f, 0x85 / 255.f, 0xF4 / 255.f, 1.f);

    skity::Rect rect = skity::Rect::MakeXYWH(10, 10, 100, 160);
    canvas_->drawRect(rect, paint);

    skity::RRect oval;
    oval.setOval(rect);
    oval.offset(40, 80);
    paint.SetFillColor(0xDB / 255.f, 0x44 / 255.f, 0x37 / 255.f, 1.f);
    canvas_->drawRRect(oval, paint);

    paint.SetFillColor(0x0F / 255.f, 0x9D / 255.f, 0x58 / 255.f, 1.f);
    canvas_->drawCircle(180, 50, 25, paint);

    rect.offset(80, 50);
    paint.SetStrokeColor(0xF4 / 255.f, 0xB4 / 255.f, 0x0, 1.f);
    paint.setStyle(skity::Paint::kStroke_Style);
    canvas_->drawRoundRect(rect, 10, 10, paint);

    canvas_->flush();
  }
  void OnDestroy() override { canvas_ = nullptr; }

 private:
  std::unique_ptr<skity::Canvas> canvas_ = {};
  float rotate = 0.f;
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
