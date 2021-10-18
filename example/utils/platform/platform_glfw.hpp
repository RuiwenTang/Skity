#ifndef EXAMPLE_UTILS_PLATFORM_PLATFORM_GLFW_HPP
#define EXAMPLE_UTILS_PLATFORM_PLATFORM_GLFW_HPP

#include "utils/vk_platform.hpp"

namespace example {

class PlatformGLFW : public Platform {
 public:
  PlatformGLFW() = default;
  ~PlatformGLFW() override = default;

  void StartUp() override;
  void CleanUp() override;
  void* CreateWindow(uint32_t windowWidth, uint32_t windowHeight,
                     const std::string& title) override;
  void StartEventLoop(VkApp* app) override;
  void DestroyWindow(void* window) override;
  void SwapBuffers(void* window) override {}
  WindowSize GetWindowSize(void* window) override;

  std::vector<const char*> GetRequiredInstanceExtensions() override;

 private:
};

}  // namespace example

#endif  // EXAMPLE_UTILS_PLATFORM_PLATFORM_GLFW_HPP
