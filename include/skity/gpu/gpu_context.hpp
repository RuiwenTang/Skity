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
  VkInstance vk_instance = {};
  VkPhysicalDevice vk_phy_device = {};
  VkDevice vk_device = {};

  GPUVkContext(GPUBackendType type, void* loader) : GPUContext(type, loader) {}
};

#endif

}  // namespace skity

#endif  // SKITY_GPU_GPU_CONTEXT_HPP