#include "src/render/hw/vk/vk_memory.hpp"

#include <cstring>
#include <skity/gpu/gpu_context.hpp>
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#include "src/logging.hpp"

namespace skity {

struct AllocatedBufferImpl : public AllocatedBuffer {
  size_t buffer_size = 0;
  VkBuffer buffer = {};
  VmaAllocation vma_allocation = {};

  VkBuffer GetBuffer() override { return buffer; }
  size_t BufferSize() override { return buffer_size; }
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
    vma_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return AllocateBufferInternal(buffer_info, vma_info);
  }

  void FreeBuffer(AllocatedBuffer* allocated_buffer) override {
    AllocatedBufferImpl* impl = (AllocatedBufferImpl*)allocated_buffer;
    vmaDestroyBuffer(vma_allocator_, impl->buffer, impl->vma_allocation);
  }

  void UploadBuffer(AllocatedBuffer* allocated_buffer, void* data,
                    size_t data_size) override;

 private:
  void InitVMA(GPUVkContext* ctx);
  void DestroyVMA(GPUVkContext* ctx);

  AllocatedBuffer* AllocateBufferInternal(
      VkBufferCreateInfo const& buffer_info,
      VmaAllocationCreateInfo const& vma_info);

 private:
  VmaAllocator vma_allocator_ = {};
};

void VKMemoryAllocatorImpl::InitVMA(GPUVkContext* ctx) {
  VmaAllocatorCreateInfo create_info{};
  create_info.vulkanApiVersion = VK_API_VERSION_1_2;
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
                                         void* data, size_t data_size) {
  auto impl = (AllocatedBufferImpl*)allocated_buffer;

  void* vma_buffer_pointer = nullptr;
  if (vmaMapMemory(vma_allocator_, impl->vma_allocation, &vma_buffer_pointer) !=
      VK_SUCCESS) {
    LOG_ERROR("Failed to map vma memory");
    return;
  }

  std::memcpy(vma_buffer_pointer, data, data_size);

  vmaUnmapMemory(vma_allocator_, impl->vma_allocation);
}

AllocatedBuffer* VKMemoryAllocatorImpl::AllocateBufferInternal(
    VkBufferCreateInfo const& buffer_info,
    VmaAllocationCreateInfo const& vma_info) {
  AllocatedBufferImpl* impl = new AllocatedBufferImpl;

  if (vmaCreateBuffer(vma_allocator_, &buffer_info, &vma_info, &impl->buffer,
                      &impl->vma_allocation, nullptr) != VK_SUCCESS) {
    LOG_ERROR("Failed to create buffer with size {}", buffer_info.size);
    delete impl;
    return nullptr;
  }
  impl->buffer_size = buffer_info.size;
  return impl;
}

std::unique_ptr<VKMemoryAllocator> VKMemoryAllocator::CreateMemoryAllocator() {
  return std::make_unique<VKMemoryAllocatorImpl>();
}

}  // namespace skity