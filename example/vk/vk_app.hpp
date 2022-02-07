#ifndef EXAMPLE_UTILS_VK_APP_HPP
#define EXAMPLE_UTILS_VK_APP_HPP

#include <memory>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <skity/gpu/gpu_vk_context.hpp>
#include <skity/skity.hpp>

namespace example {

struct ImageWrapper {
  VkImage image = {};
  VkImageView image_view = {};
  VkDeviceMemory memory = {};
  VkFormat format = {};
};

class VkApp : public skity::GPUVkContext {
 public:
  VkApp(int32_t width, int32_t height, std::string name,
        glm::vec4 const& clear_color = {0.3f, 0.4f, 0.5f, 1.f});
  virtual ~VkApp();

  void Run();

  int32_t ScreenWidth() const { return width_; }
  int32_t ScreenHeight() const { return height_; }

  VkInstance GetInstance() override { return Instance(); }
  VkPhysicalDevice GetPhysicalDevice() override { return PhysicalDevice(); }
  VkDevice GetDevice() override { return Device(); }
  VkExtent2D GetFrameExtent() override { return FrameExtent(); }
  VkCommandBuffer GetCurrentCMD() override { return CurrentCMDBuffer(); }
  VkRenderPass GetRenderPass() override { return RenderPass(); }
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr() override {
    return &vkGetInstanceProcAddr;
  }

  uint32_t GetSwapchainBufferCount() override { return SwapchinImageCount(); }

  uint32_t GetCurrentBufferIndex() override { return CurrentFrameIndex(); }

  VkQueue GetGraphicQueue() override { return GraphicQueue(); }

  VkQueue GetComputeQueue() override { return ComputeQueue(); }

  uint32_t GetGraphicQueueIndex() override { return GraphicQueueIndex(); }

  uint32_t GetComputeQueueIndex() override { return ComputeQueueIndex(); }

  VkSampleCountFlagBits GetSampleCount() override { return vk_sample_count_; }

  VkFormat GetDepthStencilFormat() override { return depth_stencil_format_; }

 protected:
  virtual void OnStart();
  virtual void OnUpdate(float elapsed_time) {}
  virtual void OnDestroy();

  void GetCursorPos(double& x, double& y);

  skity::Canvas* GetCanvas() const { return canvas_.get(); }

  float ScreenDensity() const { return density_; }

  VkInstance Instance() const { return vk_instance_; }
  VkPhysicalDevice PhysicalDevice() const { return vk_phy_device_; }
  VkDevice Device() const { return vk_device_; }
  VkExtent2D FrameExtent() const { return swap_chain_extend_; }
  VkCommandBuffer CurrentCMDBuffer() const {
    return cmd_buffers_[current_frame_];
  }
  VkRenderPass RenderPass() const { return render_pass_; }

  uint32_t SwapchinImageCount() const { return swap_chain_image_views.size(); }
  uint32_t CurrentFrameIndex() const { return current_frame_; }
  VkQueue GraphicQueue() const { return vk_graphic_queue_; }
  VkQueue ComputeQueue() const { return vk_compute_queue_; }
  uint32_t GraphicQueueIndex() const { return graphic_queue_index_; }
  uint32_t ComputeQueueIndex() const { return compute_queue_index_; }

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
  uint32_t GetMemoryType(uint32_t type_bits, VkMemoryPropertyFlags properties);

 private:
  int32_t width_ = 0;
  int32_t height_ = 0;
  float density_ = 1.f;
  std::string window_name_ = {};
  glm::vec4 clear_color_ = {};
  GLFWwindow* window_ = {};
  VkInstance vk_instance_ = {};
  VkSurfaceKHR vk_surface_ = {};
  VkDebugUtilsMessengerEXT vk_debug_messenger_ = {};
  VkPhysicalDevice vk_phy_device_ = {};
  VkSampleCountFlagBits vk_sample_count_ = VK_SAMPLE_COUNT_1_BIT;
  VkDevice vk_device_ = {};
  uint32_t graphic_queue_index_ = {};
  uint32_t present_queue_index_ = {};
  uint32_t compute_queue_index_ = {};
  VkQueue vk_graphic_queue_ = {};
  VkQueue vk_present_queue_ = {};
  VkQueue vk_compute_queue_ = {};
  VkSwapchainKHR vk_swap_chain_ = {};
  VkFormat swap_chain_format_ = {};
  VkFormat depth_stencil_format_ = {};
  VkExtent2D swap_chain_extend_ = {};
  std::vector<VkImageView> swap_chain_image_views = {};
  std::vector<ImageWrapper> stencil_image_ = {};
  std::vector<ImageWrapper> sampler_image_ = {};
  std::vector<VkFramebuffer> swap_chain_frame_buffers_ = {};
  VkCommandPool cmd_pool_ = {};
  std::vector<VkCommandBuffer> cmd_buffers_ = {};
  std::vector<VkFence> cmd_fences_ = {};
  std::vector<VkSemaphore> present_semaphore_ = {};
  std::vector<VkSemaphore> render_semaphore_ = {};
  VkRenderPass render_pass_ = {};
  uint32_t current_frame_ = {};
  uint32_t frame_index_ = 0;

  std::unique_ptr<skity::Canvas> canvas_ = {};
};

}  // namespace example

#endif  // EXAMPLE_UTILS_VK_APP_HPP
