#ifndef SKITY_SRC_RENDER_HW_VK_VK_FRAMEBUFFER_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_FRAMEBUFFER_HPP

#include <vulkan/vulkan.h>

#include <skity/gpu/gpu_context.hpp>
#include <vector>

namespace skity {

class VKMemoryAllocator;
struct AllocatedBuffer;

// helper class to hold descriptor set buffer using per frame
class VKFrameBuffer {
 public:
  VKFrameBuffer(VKMemoryAllocator* allocator);
  ~VKFrameBuffer() = default;

  void Init(GPUVkContext* ctx);
  void Destroy(GPUVkContext* ctx);

  void FrameBegin(GPUVkContext* ctx);

  // allocate VkBuffer for set 0 data binding
  AllocatedBuffer* ObtainTransformBuffer();

  // allocate descriptor set for set 0
  VkDescriptorSet ObtainTransformDataSet(GPUVkContext* ctx,
                                         VkDescriptorSetLayout layout);

 private:
  void AppendUniformBufferPool(GPUVkContext* ctx);

 private:
  VKMemoryAllocator* allocator_ = {};
  std::vector<VkDescriptorPool> uniform_buffer_pool_ = {};
  int32_t current_uniform_pool_index = -1;

  std::vector<AllocatedBuffer*> transform_buffer_ = {};
  int32_t current_transform_buffer_index = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_FRAMEBUFFER_HPP