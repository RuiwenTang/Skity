#include "utils/platform/platform_glfw.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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
};

std::unique_ptr<Platform> Platform::CreatePlatform() {
  return std::make_unique<PlatformWIN>();
}

}  // namespace example
