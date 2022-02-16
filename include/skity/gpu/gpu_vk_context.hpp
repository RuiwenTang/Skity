#ifndef SKITY_GPU_GPU_VK_CONTEXT_HPP
#define SKITY_GPU_GPU_VK_CONTEXT_HPP

#include <skity/config.hpp>
#include <skity/gpu/gpu_context.hpp>

#ifdef SKITY_VULKAN

#include <vulkan/vulkan.h>

namespace skity {

/**
 * @struct GPUVkContext
 * @note This struct is experimental and may change or delete in future
 *
 *  Struct used to create Vulkan backend.
 *  **Skity** not create swapchain or framebuffer and renderpass. All this is
 * created by outside user.
 *
 */
struct GPUVkContext : public GPUContext {
  /**
   * @brief Construct a new GPUVkContext object
   *
   * @param loader This is a pointer to **vkGetDeviceProcAddr**
   */
  GPUVkContext(void* loader) : GPUContext(GPUBackendType::kVulkan, loader) {}

  /**
   * @brief Get the VkInstance object for internal usage
   *
   * @return VkInstance
   */
  virtual VkInstance GetInstance() = 0;

  /**
   * @brief Get the VkPhysicalDevice object for internal use.
   *
   * @return VkPhysicalDevice
   */
  virtual VkPhysicalDevice GetPhysicalDevice() = 0;

  /**
   * @brief Get the Physical Device Features info
   *
   * @return VkPhysicalDeviceFeatures
   */
  virtual VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures() = 0;

  /**
   * @brief Get the VkDevice object for internal use
   *
   * @return VkDevice
   */
  virtual VkDevice GetDevice() = 0;

  /**
   * @brief Get the Framebuffer Image size info.
   *
   * @return VkExtent2D
   */
  virtual VkExtent2D GetFrameExtent() = 0;

  /**
   * @brief Get the VkCommandBuffer associate to the current Framebuffer
   *
   * @return VkCommandBuffer
   */
  virtual VkCommandBuffer GetCurrentCMD() = 0;

  /**
   * @brief Get the VkRenderPass object associate to the current Framebuffer
   *
   * @return VkRenderPass
   */
  virtual VkRenderPass GetRenderPass() = 0;

  /**
   * @brief Get the Instance Proc Addr function.
   *
   * @return PFN_vkGetInstanceProcAddr
   */
  virtual PFN_vkGetInstanceProcAddr GetInstanceProcAddr() = 0;

  /**
   * @brief Get the count of Swapchain image.
   *
   * @return uint32_t
   */
  virtual uint32_t GetSwapchainBufferCount() = 0;

  /**
   * @brief Get the Current Buffer Index number
   *
   * @return uint32_t
   */
  virtual uint32_t GetCurrentBufferIndex() = 0;

  /**
   * @brief Get the VkQueue used for graphic command submition
   *
   * @return VkQueue
   */
  virtual VkQueue GetGraphicQueue() = 0;

  /**
   * @brief Get the VkQueue used for compute command submition
   *
   * @return VkQueue
   */
  virtual VkQueue GetComputeQueue() = 0;

  /**
   * @brief Get the graphic queue family index
   *
   * @return uint32_t
   */
  virtual uint32_t GetGraphicQueueIndex() = 0;

  /**
   * @brief Get the Compute Queue family index
   *
   * @return uint32_t
   */
  virtual uint32_t GetComputeQueueIndex() = 0;

  /**
   * @brief Get the Sample Count of current Vulkan context
   *
   * @return VkSampleCountFlagBits
   */
  virtual VkSampleCountFlagBits GetSampleCount() = 0;

  /**
   * @brief Get the Depth Stencil Format which current Vulkan context supported
   *
   * @return VkFormat
   */
  virtual VkFormat GetDepthStencilFormat() = 0;

  /**
   * @brief Get the Swapchain Surface Transform flag
   * @note  By default, canvas does not need to handle surface transform, and
   *        this function should return `VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR`.
   *        But in Android or other mobile platforms, the surface orientation
   *        could be changed, and the pipeline MVP matrix need to make the
   *        adjustment.
   *
   * @return VkSurfaceTransformFlagBitsKHR
   */
  virtual VkSurfaceTransformFlagBitsKHR GetSurfaceTransform() = 0;
};

}  // namespace skity

#endif  // SKITY_VULKAN

#endif  // SKITY_GPU_GPU_VK_CONTEXT_HPP