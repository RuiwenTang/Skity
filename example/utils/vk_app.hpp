#ifndef EXAMPLE_UTILS_VK_APP_HPP
#define EXAMPLE_UTILS_VK_APP_HPP

#include <memory>
#include <string>
#include <vector>
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
  vk::Device GetVkDevice() { return vk_device_.get(); }
  vk::SurfaceKHR GetVkSurface() { return vk_surface_.get(); }
  vk::CommandBuffer GetVkCommandBuffer() { return vk_command_buffer_.get(); }
  vk::CommandPool GetVkCommandPool() { return vk_command_pool_.get(); }
  vk::Extent2D GetFrameExtend() { return vk_frame_extent_; }
  vk::Format GetFrameFormat() { return vk_color_attachment_format_; }
  vk::DispatchLoaderStatic const& GetVkDispatch() { return vk_dispatch_; }
  vk::Framebuffer GetCurrentFramebuffer() {
    return vk_swap_chain_frame_buffer_[vk_current_frame_index_].get();
  }
  vk::RenderPass GetAppRenderPass() { return vk_render_pass_.get(); }

  // util function may move to other class
  uint32_t FindMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);

  virtual void OnCreate() {}
  virtual void OnUpdate(float elapsed_time) {}
  virtual void OnDestroy() {}

 private:
  void CreateVkInstance();
  void PickPhysicalDevice();
  void CreateSurface();
  void CreateLogicalDevice();
  void CreateSwapChain();
  void CreateSwapChainImageView();
  void CreateCommandPoolAndBuffer();
  void CreateRenderPass();
  void CreateFramebuffer();
  void CreateSyncObject();

  void BeginForDraw();
  void EndForDraw();

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
  vk::UniqueSwapchainKHR vk_swap_chain_;
  vk::Extent2D vk_frame_extent_;
  vk::Queue vk_graphic_queue_;
  vk::Queue vk_present_queue_;
  vk::Format vk_color_attachment_format_ = vk::Format::eUndefined;
  std::vector<vk::UniqueImageView> vk_swap_chain_image_view_;
  vk::UniqueCommandPool vk_command_pool_;
  vk::UniqueCommandBuffer vk_command_buffer_;
  vk::UniqueRenderPass vk_render_pass_;
  std::vector<vk::UniqueFramebuffer> vk_swap_chain_frame_buffer_;
  uint32_t vk_current_frame_index_ = 0;
  vk::UniqueSemaphore vk_image_acquired_semaphore_;
  vk::UniqueFence vk_draw_fence_;
};

}  // namespace example

#endif  // EXAMPLE_UTILS_VK_APP_HPP
