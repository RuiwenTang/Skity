#include "src/render/hw/vk/vk_memory.hpp"

#include <cstring>
#include <skity/gpu/gpu_context.hpp>
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

struct AllocatedBufferImpl : public AllocatedBuffer {
  size_t buffer_size = 0;
  VkBuffer buffer = {};
  VmaAllocation vma_allocation = {};
  VmaAllocationInfo vma_allocation_info = {};

  AllocatedBufferImpl() = default;
  ~AllocatedBufferImpl() override = default;

  VkBuffer GetBuffer() override { return buffer; }
  size_t BufferSize() override { return buffer_size; }
};

struct AllocatedImageImpl : public AllocatedImage {
  VkImage image = {};
  VkImageLayout layout = {};
  VkFormat format = {};
  VkExtent3D extent = {};
  VmaAllocation vma_allocation = {};

  AllocatedImageImpl() = default;
  ~AllocatedImageImpl() override = default;

  VkImage GetImage() override { return image; }
  VkImageLayout GetCurrentLayout() override { return layout; }
  VkFormat GetImageFormat() override { return format; }
  VkExtent3D GetImageExtent() override { return extent; }
};

class VKMemoryAllocatorImpl : public VKMemoryAllocator {
 public:
  VKMemoryAllocatorImpl() = default;
  ~VKMemoryAllocatorImpl() override = default;

  void Init(GPUVkContext* ctx) override { InitVMA(ctx); }

  void Destroy(GPUVkContext* ctx) override { DestroyVMA(ctx); }

  AllocatedBuffer* AllocateVertexBuffer(size_t buffer_size) override {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vma_info{};
    vma_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vma_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return AllocateBufferInternal(buffer_info, vma_info);
  }

  AllocatedBuffer* AllocateIndexBuffer(size_t buffer_size) override {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo vma_info{};
    vma_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vma_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return AllocateBufferInternal(buffer_info, vma_info);
  }

  AllocatedBuffer* AllocateUniformBuffer(size_t buffer_size) override {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo vma_info{};
    vma_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vma_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    vma_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return AllocateBufferInternal(buffer_info, vma_info);
  }

  AllocatedBuffer* AllocateStageBuffer(size_t buffer_size) override {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo vma_info{};
    vma_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vma_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    return AllocateBufferInternal(buffer_info, vma_info);
  }

  AllocatedImage* AllocateImage(VkFormat format, VkExtent3D extent,
                                VkImageAspectFlags flags) override {
    VkImageCreateInfo image_info =
        VKUtils::ImageCreateInfo(format, flags, extent);

    VmaAllocationCreateInfo vma_info{};
    vma_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vma_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    return AllocateImageInternal(image_info, vma_info);
  }

  void FreeBuffer(AllocatedBuffer* allocated_buffer) override {
    AllocatedBufferImpl* impl = (AllocatedBufferImpl*)allocated_buffer;
    vmaDestroyBuffer(vma_allocator_, impl->buffer, impl->vma_allocation);
  }

  void FreeImage(AllocatedImage* allocated_image) override {
    AllocatedImageImpl* impl = (AllocatedImageImpl*)allocated_image;
    vmaDestroyImage(vma_allocator_, impl->image, impl->vma_allocation);
  }

  void UploadBuffer(AllocatedBuffer* allocated_buffer, void* data,
                    size_t data_size, size_t offset) override;

  void TransferImageLayout(VkCommandBuffer cmd, AllocatedImage* image,
                           VkImageSubresourceRange range,
                           VkImageLayout old_layout,
                           VkImageLayout new_layout) override;

  void CopyBufferToImage(VkCommandBuffer cmd, AllocatedBuffer* buffer,
                         AllocatedImage* image,
                         VkBufferImageCopy const& copy_region) override;

 private:
  void InitVMA(GPUVkContext* ctx);
  void DestroyVMA(GPUVkContext* ctx);

  AllocatedBuffer* AllocateBufferInternal(
      VkBufferCreateInfo const& buffer_info,
      VmaAllocationCreateInfo const& vma_info);

  AllocatedImage* AllocateImageInternal(
      VkImageCreateInfo const& image_info,
      VmaAllocationCreateInfo const& vma_info);

 private:
  VmaAllocator vma_allocator_ = {};
};

void VKMemoryAllocatorImpl::InitVMA(GPUVkContext* ctx) {
  VmaAllocatorCreateInfo create_info{};
  create_info.vulkanApiVersion = VK_API_VERSION_1_0;
  create_info.physicalDevice = ctx->GetPhysicalDevice();
  create_info.device = ctx->GetDevice();
  create_info.instance = ctx->GetInstance();

  VmaVulkanFunctions vk_functions{};
  vk_functions.vkGetInstanceProcAddr = ctx->GetInstanceProcAddr();
  vk_functions.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)ctx->proc_loader;

  create_info.pVulkanFunctions = &vk_functions;

  if (vmaCreateAllocator(&create_info, &this->vma_allocator_) != VK_SUCCESS) {
    LOG_ERROR("Failed to create vma allocator");
  }
}

void VKMemoryAllocatorImpl::DestroyVMA(GPUVkContext* ctx) {
  vmaDestroyAllocator(vma_allocator_);
}

void VKMemoryAllocatorImpl::UploadBuffer(AllocatedBuffer* allocated_buffer,
                                         void* data, size_t data_size,
                                         size_t offset) {
  auto impl = (AllocatedBufferImpl*)allocated_buffer;

  if (impl->vma_allocation_info.pMappedData) {
    void* p = ((char*)impl->vma_allocation_info.pMappedData + offset);
    std::memcpy(p, data, data_size);
    return;
  }

  void* vma_buffer_pointer = nullptr;
  if (vmaMapMemory(vma_allocator_, impl->vma_allocation, &vma_buffer_pointer) !=
      VK_SUCCESS) {
    LOG_ERROR("Failed to map vma memory");
    return;
  }
  void* p = ((char*)vma_buffer_pointer + offset);
  std::memcpy(p, data, data_size);

  vmaUnmapMemory(vma_allocator_, impl->vma_allocation);
}

void VKMemoryAllocatorImpl::TransferImageLayout(VkCommandBuffer cmd,
                                                AllocatedImage* image,
                                                VkImageSubresourceRange range,
                                                VkImageLayout old_layout,
                                                VkImageLayout new_layout) {
  AllocatedImageImpl* impl = (AllocatedImageImpl*)image;
  VKUtils::SetImageLayout(cmd, impl->image, range.aspectMask, old_layout,
                          new_layout, range);
  impl->layout = new_layout;
}

void VKMemoryAllocatorImpl::CopyBufferToImage(
    VkCommandBuffer cmd, AllocatedBuffer* buffer, AllocatedImage* image,
    VkBufferImageCopy const& copy_region) {
  AllocatedBufferImpl* buffer_impl = (AllocatedBufferImpl*)buffer;
  AllocatedImageImpl* image_impl = (AllocatedImageImpl*)image;

  VK_CALL(vkCmdCopyBufferToImage, cmd, buffer_impl->buffer, image_impl->image,
          image_impl->layout, 1, &copy_region);
}

AllocatedBuffer* VKMemoryAllocatorImpl::AllocateBufferInternal(
    VkBufferCreateInfo const& buffer_info,
    VmaAllocationCreateInfo const& vma_info) {
  AllocatedBufferImpl* impl = new AllocatedBufferImpl;

  if (vmaCreateBuffer(vma_allocator_, &buffer_info, &vma_info, &impl->buffer,
                      &impl->vma_allocation,
                      &impl->vma_allocation_info) != VK_SUCCESS) {
    LOG_ERROR("Failed to create buffer with size {}", buffer_info.size);
    delete impl;
    return nullptr;
  }
  impl->buffer_size = buffer_info.size;
  return impl;
}

AllocatedImage* VKMemoryAllocatorImpl::AllocateImageInternal(
    VkImageCreateInfo const& image_info,
    VmaAllocationCreateInfo const& vma_info) {
  AllocatedImageImpl* impl = new AllocatedImageImpl;

  if (vmaCreateImage(vma_allocator_, &image_info, &vma_info, &impl->image,
                     &impl->vma_allocation, nullptr) != VK_SUCCESS) {
    LOG_ERROR("Failed allocate image with format {} and extent: [{}, {}]",
              image_info.format, image_info.extent.width,
              image_info.extent.height);
    delete impl;
    return nullptr;
  }

  impl->format = image_info.format;
  impl->extent = image_info.extent;
  impl->layout = VK_IMAGE_LAYOUT_UNDEFINED;

  return impl;
}

std::unique_ptr<VKMemoryAllocator> VKMemoryAllocator::CreateMemoryAllocator() {
  return std::make_unique<VKMemoryAllocatorImpl>();
}

}  // namespace skity
