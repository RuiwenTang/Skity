#ifndef EXAMPLE_UTILS_VK_APP_HPP
#define EXAMPLE_UTILS_VK_APP_HPP

#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

namespace example {

class Platform;
class VkApp {
 public:
  VkApp(int32_t width, int32_t height, std::string name);
  virtual ~VkApp();

  void Run();

  int32_t ScreenWidth() const { return width_; }
  int32_t ScreenHeight() const { return height_; }

  Platform* GetPlatform() { return platform_.get(); }
  void* GetWindowHandle() { return window_; }

  void Update(float elapsed_time);

 protected:
  virtual void OnCreate() {}
  virtual void OnUpdate(float elapsed_time) {}
  virtual void OnDestroy() {}

 private:
  void CreateVkInstance();
  void PickPhysicalDevice();
  void CreateSurface();
  void CreateLogicalDevice();

 private:
  int32_t width_ = 0;
  int32_t height_ = 0;
  std::string window_name_;
  void* window_ = nullptr;
  std::unique_ptr<Platform> platform_;
  vk::DispatchLoaderStatic vk_dispatch_;
  vk::UniqueInstance vk_instance_;
  vk::UniqueSurfaceKHR vk_surface_;
  vk::UniqueDebugUtilsMessengerEXT vk_debug_messenger_;
  vk::PhysicalDevice vk_physical_device_;
  int32_t vk_graphic_queue_index_ = -1;
  int32_t vk_present_queue_index_ = -1;
  vk::UniqueDevice vk_device_;
};
}  // namespace example

#endif  // EXAMPLE_UTILS_VK_APP_HPP
