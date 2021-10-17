#ifndef EXAMPLE_UTILS_PLATFORM_HPP
#define EXAMPLE_UTILS_PLATFORM_HPP

#include <array>
#include <memory>
#include <string>

namespace example {

class App;
class Platform {
  using WindowSize = std::array<int32_t, 2>;
  virtual ~Platform() = default;

  virtual void StartUp() = 0;
  virtual void CleanUp() = 0;
  virtual void* CreateWindow(uint32_t windowWidth, uint32_t windowHeight,
                             const std::string& title, bool fullScreen) = 0;
  virtual void StartEventLoop(App* app) = 0;
  virtual void DestroyWindow(void* window) = 0;
  virtual void* GetWindowSurface(void* window) = 0;
  virtual void SwapBuffers(void* window) = 0;
  virtual WindowSize GetWindowSize(void* window) = 0;

  static std::unique_ptr<Platform> GetPlatform();
};

}  // namespace example

#endif  // EXAMPLE_UTILS_PLATFORM_HPP
