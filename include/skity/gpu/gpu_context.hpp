#ifndef SKITY_GPU_GPU_CONTEXT_HPP
#define SKITY_GPU_GPU_CONTEXT_HPP

#ifdef SKITY_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace skity {

enum class GPUBackendType {
  kNone,
  kOpenGL,
#ifdef SKITY_VULKAN
  kVulkan,
#endif
};

struct GPUContext {
  GPUBackendType type = GPUBackendType::kNone;
  void* proc_loader = nullptr;

  GPUContext(GPUBackendType type, void* proc_loader)
      : type(type), proc_loader(proc_loader) {}
};

#ifdef SKITY_VULKAN

struct GPUVkContext : public GPUContext {
  GPUVkContext(void* loader) : GPUContext(GPUBackendType::kVulkan, loader) {}

  virtual VkInstance GetInstance() = 0;
  virtual VkPhysicalDevice GetPhysicalDevice() = 0;
  virtual VkDevice GetDevice() = 0;
  virtual VkExtent2D GetFrameExtent() = 0;
  virtual VkCommandBuffer GetCurrentCMD() = 0;
};

#endif

}  // namespace skity

#endif  // SKITY_GPU_GPU_CONTEXT_HPP