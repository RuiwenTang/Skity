#include "utils/platform/platform_glfw.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

namespace example {

class PlatformWIN : public PlatformGLFW {
 public:
  PlatformWIN() = default;
  ~PlatformWIN() override = default;

  void* GetWindowSurface(void* window) override {
    if (window == nullptr) {
      return nullptr;
    }

    return glfwGetWin32Window(reinterpret_cast<GLFWwindow*>(window));
  }

  void* CreateSurface(void* instance, void* window) override {
    vk::Win32SurfaceCreateInfoKHR create_info{
        {}, nullptr, reinterpret_cast<HWND>(GetWindowSurface(window))};

    vk::Instance vk_instance{static_cast<VkInstance>(instance)};

    auto create_ret = vk_instance.createWin32SurfaceKHR(create_info);

    if (create_ret.result != vk::Result::eSuccess) {
      return nullptr;
    } else {
      return create_ret.value;
    }
  }

  std::vector<const char*> GetRequiredInstanceExtensions() override {
    auto extensions = PlatformGLFW::GetRequiredInstanceExtensions();

    extensions.emplace_back("VK_KHR_win32_surface");

    return extensions;
  }
};

std::unique_ptr<Platform> Platform::CreatePlatform() {
  return std::make_unique<PlatformWIN>();
}

}  // namespace example
