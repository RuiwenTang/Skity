#include "utils/vk_app.hpp"

#include "utils/vk_platform.hpp"

namespace example {

VkApp::VkApp(int32_t width, int32_t height, std::string name)

    : width_(width),
      height_(height),
      window_name_(std::move(name)),
      window_(nullptr),
      platform_(Platform::CreatePlatform()) {}

VkApp::~VkApp() = default;

void VkApp::Run() {
  platform_->StartUp();

  window_ = platform_->CreateWindow(width_, height_, window_name_);
  if (!window_) {
    exit(-1);
  }

  this->OnCreate();

  platform_->StartEventLoop(this);

  this->OnDestroy();

  platform_->CleanUp();
  platform_->DestroyWindow(window_);
  platform_->CleanUp();
}

void VkApp::Update(float elapsed_time) {
  this->OnUpdate(elapsed_time);
  platform_->SwapBuffers(window_);
}

}  // namespace example
