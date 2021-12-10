#include "src/render/hw/vk/vk_memory.hpp"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

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

  void Init(GPUVkContext* ctx) override { InitVMA(); }

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

 private:
  void InitVMA();

  AllocatedBuffer* AllocateBufferInternal(
      VkBufferCreateInfo const& buffer_info,
      VmaAllocationCreateInfo const& vma_info);

 private:
  VmaAllocator vm_allocator_;
};

void VKMemoryAllocatorImpl::InitVMA() {}

AllocatedBuffer* VKMemoryAllocatorImpl::AllocateBufferInternal(
    VkBufferCreateInfo const& buffer_info,
    VmaAllocationCreateInfo const& vma_info) {
  return nullptr;
}

std::unique_ptr<VKMemoryAllocator> VKMemoryAllocator::CreateMemoryAllocator() {
  return std::make_unique<VKMemoryAllocatorImpl>();
}

}  // namespace skity