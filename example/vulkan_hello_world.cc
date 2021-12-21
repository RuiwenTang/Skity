#include <spdlog/spdlog.h>

#include <cstring>
#include <example_config.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <skity/skity.hpp>
#include <vector>

#include "example_config.hpp"
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

  VkQueue GetGraphicQueue() override { return VkApp::GraphicQueue(); }

  uint32_t GetGraphicQueueIndex() override {
    return VkApp::GraphicQueueIndex();
  }

 protected:
  void OnStart() override {
    canvas_ = skity::Canvas::MakeHardwareAccelationCanvas(
        800, 800, ScreenDensity(), this);

    canvas_->setDefaultTypeface(
        skity::Typeface::MakeFromFile(EXAMPLE_DEFAULT_FONT));

    auto data = skity::Data::MakeFromFileName(EXAMPLE_IMAGE_ROOT "/image1.jpg");

    auto codec = skity::Codec::MakeFromData(data);

    if (!codec) {
      spdlog::error("Failed create codec for example jpg image!");
      return;
    }
    codec->SetData(data);
    pixmap_ = codec->Decode();

    if (!pixmap_ || pixmap_->RowBytes() == 0) {
      spdlog::error("Failed to decode example jpg image!");
      pixmap_ = nullptr;
    }
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

    skity::Path clip_path;
    clip_path.addRect(skity::Rect::MakeXYWH(520, 400, 100, 80));

    paint.setStrokeWidth(2.f);
    paint.setColor(skity::Color_BLUE);
    paint.setStrokeCap(skity::Paint::kRound_Cap);

    rotate += 1.f;
    if (rotate >= 360.f) {
      rotate = 0.f;
    }

    canvas_->save();
    canvas_->rotate(rotate, 570, 440);

    canvas_->drawPath(clip_path, paint);
    canvas_->clipPath(clip_path);
    canvas_->rotate(-rotate, 570, 440);

    paint.setStrokeWidth(10.f);
    paint.setColor(skity::ColorSetARGB(64, 255, 0, 0));
    {
      skity::Path temp_path;
      temp_path.moveTo(500, 380);
      temp_path.lineTo(600, 450);
      temp_path.lineTo(600, 400);
      canvas_->drawPath(temp_path, paint);
    }
    canvas_->restore();

    paint.setColor(skity::Color_LTGRAY);
    canvas_->drawLine(500, 420, 600, 470, paint);

    {
      skity::Point center{400, 400, 0, 1.f};
      skity::Vec4 radialColors[] = {skity::Vec4{1.f, 1.f, 1.f, 1.f},
                                    skity::Vec4{0.f, 0.f, 0.f, 1.f}};
      float pts[] = {0.f, 1.f};
      auto rgs =
          skity::Shader::MakeRadial(center, 120.f, radialColors, nullptr, 2);
      paint.setShader(rgs);
    }

    paint.setStyle(skity::Paint::kFill_Style);
    canvas_->drawCircle(400, 400, 100, paint);

    paint.setShader(nullptr);
    if (pixmap_) {
      paint.setShader(skity::Shader::MakeShader(pixmap_));

      canvas_->drawRect(skity::Rect::MakeXYWH(300, 100, 150, 150), paint);
    }

    paint.setShader(nullptr);
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setTextSize(20.f);
    canvas_->drawSimpleText2("12ATx", 500, 300, paint);

    canvas_->flush();
  }
  void OnDestroy() override { canvas_ = nullptr; }

 private:
  std::unique_ptr<skity::Canvas> canvas_ = {};
  std::shared_ptr<skity::Pixmap> pixmap_ = {};
  float rotate = 0.f;
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
