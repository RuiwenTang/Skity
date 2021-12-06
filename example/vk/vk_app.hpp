#ifndef EXAMPLE_UTILS_VK_APP_HPP
#define EXAMPLE_UTILS_VK_APP_HPP

#include <memory>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace example {

class VkApp {
 public:
  VkApp(int32_t width, int32_t height, std::string name);
  virtual ~VkApp();

  void Run();

  int32_t ScreenWidth() const { return width_; }
  int32_t ScreenHeight() const { return height_; }

 protected:
  virtual void OnStart() {}
  virtual void OnUpdate(float elapsed_time) {}
  virtual void OnDestroy() {}

  VkInstance Instance() const { return vk_instance_; }
  VkPhysicalDevice PhysicalDevice() const { return vk_phy_device_; }
  VkDevice Device() const { return vk_device_; }

 private:
  void SetupVkContext();
  void Loop();
  void CleanUp();

  // vulkan utils function
  void CreateVkInstance();
  void CreateVkSurface();
  void PickPhysicalDevice();
  void CreateVkDevice();
  void CreateSwapChain();
  void CreateSwapChainImageViews();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void CreateSyncObjects();
  void CreateRenderPass();
  void CreateFramebuffers();

 private:
  int32_t width_ = 0;
  int32_t height_ = 0;
  std::string window_name_ = {};
  GLFWwindow* window_ = {};
  VkInstance vk_instance_ = {};
  VkSurfaceKHR vk_surface_ = {};
  VkDebugUtilsMessengerEXT vk_debug_messenger_ = {};
  VkPhysicalDevice vk_phy_device_ = {};
  VkDevice vk_device_ = {};
  uint32_t graphic_queue_index_ = {};
  uint32_t present_queue_index_ = {};
  VkQueue vk_graphic_queue_ = {};
  VkQueue vk_present_queue_ = {};
  VkSwapchainKHR vk_swap_chain_ = {};
  VkFormat swap_chain_format_ = {};
  VkExtent2D swap_chain_extend_ = {};
  std::vector<VkImageView> swap_chain_image_views = {};
  std::vector<VkFramebuffer> swap_chain_frame_buffers_ = {};
  VkCommandPool cmd_pool_ = {};
  std::vector<VkCommandBuffer> cmd_buffers_ = {};
  std::vector<VkFence> cmd_fences_ = {};
  VkSemaphore present_semaphore_ = {};
  VkSemaphore render_semaphore_ = {};
  VkRenderPass render_pass_ = {};
  uint32_t current_frame_ = {};
};

}  // namespace example

#endif  // EXAMPLE_UTILS_VK_APP_HPP
