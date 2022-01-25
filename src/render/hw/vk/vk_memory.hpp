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

struct AllocatedImage {
  AllocatedImage() = default;
  virtual ~AllocatedImage() = default;

  virtual VkImage GetImage() = 0;
  virtual VkImageLayout GetCurrentLayout() = 0;
  virtual VkFormat GetImageFormat() = 0;
  virtual VkExtent3D GetImageExtent() = 0;
};

class VKMemoryAllocator {
 public:
  VKMemoryAllocator() = default;
  virtual ~VKMemoryAllocator() = default;

  virtual void Init(GPUVkContext* ctx) = 0;

  virtual void Destroy(GPUVkContext* ctx) = 0;

  virtual AllocatedBuffer* AllocateVertexBuffer(size_t buffer_size) = 0;

  virtual AllocatedBuffer* AllocateIndexBuffer(size_t buffer_size) = 0;

  virtual AllocatedBuffer* AllocateUniformBuffer(size_t buffer_size) = 0;

  virtual AllocatedBuffer* AllocateStageBuffer(size_t buffer_size) = 0;

  virtual AllocatedImage* AllocateImage(VkFormat format, VkExtent3D extent,
                                        VkImageUsageFlags flags) = 0;

  virtual void FreeBuffer(AllocatedBuffer* allocated_buffer) = 0;

  virtual void FreeImage(AllocatedImage* allocated_image) = 0;

  virtual void UploadBuffer(AllocatedBuffer* allocated_buffer, void* data,
                            size_t data_size, size_t offset = 0) = 0;

  virtual void TransferImageLayout(VkCommandBuffer cmd, AllocatedImage* image,
                                   VkImageSubresourceRange range,
                                   VkImageLayout old_layout,
                                   VkImageLayout new_layout) = 0;

  virtual void TransferImageLayout(AllocatedImage* image,
                                   VkImageLayout new_layout) = 0;

  virtual void CopyBufferToImage(VkCommandBuffer cmd, AllocatedBuffer* buffer,
                                 AllocatedImage* image,
                                 VkBufferImageCopy const& copy_region) = 0;

  static std::unique_ptr<VKMemoryAllocator> CreateMemoryAllocator();
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_MEMORY_HPP