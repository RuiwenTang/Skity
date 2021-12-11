#ifndef SKITY_SRC_RENDER_HW_VK_MEMORY_HPP
#define SKITY_SRC_RENDER_HW_VK_MEMORY_HPP

#include <vulkan/vulkan.h>

#include <memory>

namespace skity {

struct GPUVkContext;

struct AllocatedBuffer {
  AllocatedBuffer() = default;
  virtual ~AllocatedBuffer() = default;

  virtual VkBuffer GetBuffer() = 0;
  virtual size_t BufferSize() = 0;
};

class VKMemoryAllocator {
 public:
  VKMemoryAllocator() = default;
  virtual ~VKMemoryAllocator() = default;

  virtual void Init(GPUVkContext* ctx) = 0;

  virtual void Destroy(GPUVkContext* ctx) = 0;

  virtual AllocatedBuffer* AllocateVertexBuffer(size_t buffer_size) = 0;

  virtual AllocatedBuffer* AllocateIndexBuffer(size_t buffer_size) = 0;

  virtual void FreeBuffer(AllocatedBuffer* allocated_buffer) = 0;

  virtual void UploadBuffer(AllocatedBuffer* allocated_buffer, void* data,
                            size_t data_size) = 0;

  static std::unique_ptr<VKMemoryAllocator> CreateMemoryAllocator();
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_MEMORY_HPP