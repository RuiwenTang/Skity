#ifndef EXAMPLE_UTILS_PLATFORM_HPP
#define EXAMPLE_UTILS_PLATFORM_HPP

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace example {

class VkApp;
class Platform {
 public:
  using WindowSize = std::array<int32_t, 2>;
  virtual ~Platform() = default;

  virtual void StartUp() = 0;
  virtual void CleanUp() = 0;
  virtual void* CreateWindow(uint32_t windowWidth, uint32_t windowHeight,
                             const std::string& title) = 0;
  virtual void* CreateSurface(void* instance,void* window) = 0;
  virtual void StartEventLoop(VkApp* app) = 0;
  virtual void DestroyWindow(void* window) = 0;
  virtual void* GetWindowSurface(void* window) = 0;
  virtual void SwapBuffers(void* window) = 0;
  virtual WindowSize GetWindowSize(void* window) = 0;
  virtual std::vector<const char*> GetRequiredInstanceExtensions() = 0;

  static std::unique_ptr<Platform> CreatePlatform();
};

}  // namespace example

#endif  // EXAMPLE_UTILS_PLATFORM_HPP
