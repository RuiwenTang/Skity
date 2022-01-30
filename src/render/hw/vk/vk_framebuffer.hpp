#ifndef SKITY_SRC_RENDER_HW_VK_VK_FRAMEBUFFER_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_FRAMEBUFFER_HPP

#include <vulkan/vulkan.h>

#include <skity/gpu/gpu_context.hpp>
#include <vector>

namespace skity {

class VKMemoryAllocator;
struct AllocatedBuffer;

// helper class to hold descriptor set buffer using per frame
class SKVkFrameBufferData {
 public:
  SKVkFrameBufferData(VKMemoryAllocator* allocator);
  ~SKVkFrameBufferData() = default;

  void Init(GPUVkContext* ctx);
  void Destroy(GPUVkContext* ctx);

  void FrameBegin(GPUVkContext* ctx);

  // allocate VkBuffer for set 0 data binding
  AllocatedBuffer* ObtainTransformBuffer();

  AllocatedBuffer* ObtainCommonSetBuffer();

  AllocatedBuffer* ObtainUniformColorBuffer();

  AllocatedBuffer* ObtainGradientBuffer();

  AllocatedBuffer* ObtainComputeInfoBuffer();

  // allocate descriptor set for set 0
  VkDescriptorSet ObtainUniformBufferSet(GPUVkContext* ctx,
                                         VkDescriptorSetLayout layout);

 private:
  void AppendUniformBufferPool(GPUVkContext* ctx);

 private:
  VKMemoryAllocator* allocator_ = {};
  std::vector<VkDescriptorPool> uniform_buffer_pool_ = {};
  int32_t current_uniform_pool_index = -1;

  std::vector<AllocatedBuffer*> transform_buffer_ = {};
  int32_t current_transform_buffer_index = -1;

  // global alpha stroke_width buffer
  std::vector<AllocatedBuffer*> common_set_buffer_ = {};
  int32_t common_set_buffer_index_ = -1;

  std::vector<AllocatedBuffer*> uniform_color_buffer_ = {};
  int32_t color_buffer_index = -1;

  std::vector<AllocatedBuffer*> gradient_info_buffer_ = {};
  int32_t gradient_info_index = -1;

  std::vector<AllocatedBuffer*> compute_info_buffer_ = {};
  int32_t compute_info_index = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_FRAMEBUFFER_HPP