#include <spdlog/spdlog.h>

#include <cstring>
#include <example_config.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <skity/skity.hpp>
#include <vector>

#include "example_config.hpp"
#include "vk/vk_app.hpp"

class HelloVulkanApp : public example::VkApp {
 public:
  HelloVulkanApp() : VkApp(800, 800, "Hello Vulkan") {}
  ~HelloVulkanApp() override = default;

 protected:
  void OnStart() override {
    example::VkApp::OnStart();

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
    GetCanvas()->drawRect(rect, paint);

    skity::RRect oval;
    oval.setOval(rect);
    oval.offset(40, 80);
    paint.SetFillColor(0xDB / 255.f, 0x44 / 255.f, 0x37 / 255.f, 1.f);
    GetCanvas()->drawRRect(oval, paint);

    paint.SetFillColor(0x0F / 255.f, 0x9D / 255.f, 0x58 / 255.f, 1.f);
    GetCanvas()->drawCircle(180, 50, 25, paint);

    rect.offset(80, 50);
    paint.SetStrokeColor(0xF4 / 255.f, 0xB4 / 255.f, 0x0, 1.f);
    paint.setStyle(skity::Paint::kStroke_Style);
    GetCanvas()->drawRoundRect(rect, 10, 10, paint);

    skity::Path clip_path;
    clip_path.addRect(skity::Rect::MakeXYWH(520, 400, 100, 80));

    paint.setStrokeWidth(2.f);
    paint.setColor(skity::Color_BLUE);
    paint.setStrokeCap(skity::Paint::kRound_Cap);

    rotate += 1.f;
    if (rotate >= 360.f) {
      rotate = 0.f;
    }

    GetCanvas()->save();
    GetCanvas()->rotate(rotate, 570, 440);

    GetCanvas()->drawPath(clip_path, paint);
    GetCanvas()->clipPath(clip_path);
    GetCanvas()->rotate(-rotate, 570, 440);

    paint.setStrokeWidth(10.f);
    paint.setColor(skity::ColorSetARGB(64, 255, 0, 0));
    {
      skity::Path temp_path;
      temp_path.moveTo(500, 380);
      temp_path.lineTo(600, 450);
      temp_path.lineTo(600, 400);
      GetCanvas()->drawPath(temp_path, paint);
    }
    GetCanvas()->restore();

    paint.setColor(skity::Color_LTGRAY);
    GetCanvas()->drawLine(500, 420, 600, 470, paint);

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
    GetCanvas()->drawCircle(400, 400, 100, paint);

    paint.setShader(nullptr);
    if (pixmap_) {
      paint.setShader(skity::Shader::MakeShader(pixmap_));

      GetCanvas()->drawRect(skity::Rect::MakeXYWH(300, 100, 150, 150), paint);
    }

    paint.setShader(nullptr);
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setTextSize(20.f);
    GetCanvas()->drawSimpleText2("12ATx", 500, 300, paint);

    GetCanvas()->flush();
  }

 private:
  std::shared_ptr<skity::Pixmap> pixmap_ = {};
  float rotate = 0.f;
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
