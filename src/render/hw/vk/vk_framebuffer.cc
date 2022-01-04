#include "src/render/hw/vk/vk_framebuffer.hpp"

#include <array>
#include <cassert>
#include <skity/geometry/point.hpp>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

#define DEFAULT_UNIFORM_SIZE_PER_POOL 1000

SKVkFrameBufferData::SKVkFrameBufferData(VKMemoryAllocator* allocator)
    : allocator_(allocator) {}

void SKVkFrameBufferData::Init(GPUVkContext* ctx) {
  AppendUniformBufferPool(ctx);
}

void SKVkFrameBufferData::Destroy(GPUVkContext* ctx) {
  for (auto buffer : transform_buffer_) {
    allocator_->FreeBuffer(buffer);
    delete buffer;
  }

  for (auto buffer : common_set_buffer_) {
    allocator_->FreeBuffer(buffer);
    delete buffer;
  }

  for (auto buffer : uniform_color_buffer_) {
    allocator_->FreeBuffer(buffer);
    delete buffer;
  }

  for (auto buffer : gradient_info_buffer_) {
    allocator_->FreeBuffer(buffer);
    delete buffer;
  }

  current_transform_buffer_index = -1;

  for (auto pool : uniform_buffer_pool_) {
    VK_CALL(vkResetDescriptorPool, ctx->GetDevice(), pool, 0);
    VK_CALL(vkDestroyDescriptorPool, ctx->GetDevice(), pool, nullptr);
  }

  uniform_buffer_pool_.clear();
  current_uniform_pool_index = -1;
}

void SKVkFrameBufferData::FrameBegin(GPUVkContext* ctx) {
  if (current_transform_buffer_index >= 0) {
    current_transform_buffer_index = 0;
  }

  if (common_set_buffer_index_ >= 0) {
    common_set_buffer_index_ = 0;
  }

  if (color_buffer_index >= 0) {
    color_buffer_index = 0;
  }

  if (gradient_info_index >= 0) {
    gradient_info_index = 0;
  }

  for (auto pool : uniform_buffer_pool_) {
    VK_CALL(vkResetDescriptorPool, ctx->GetDevice(), pool, 0);
  }

  if (current_uniform_pool_index >= 0) {
    current_uniform_pool_index = 0;
  }
}

void SKVkFrameBufferData::AppendUniformBufferPool(GPUVkContext* ctx) {
  std::array<VkDescriptorPoolSize, 2> pool_size{};
  pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size[0].descriptorCount = DEFAULT_UNIFORM_SIZE_PER_POOL;
  pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size[1].descriptorCount = DEFAULT_UNIFORM_SIZE_PER_POOL;

  auto create_info = VKUtils::DescriptorPoolCreateInfo(
      pool_size.size(), pool_size.data(), DEFAULT_UNIFORM_SIZE_PER_POOL);

  VkDescriptorPool pool = VK_NULL_HANDLE;

  if (VK_CALL(vkCreateDescriptorPool, ctx->GetDevice(), &create_info, nullptr,
              &pool) != VK_SUCCESS) {
    LOG_ERROR("Failed to create descriptor pool for uniform buffer!");
    assert(false);
  }

  uniform_buffer_pool_.emplace_back(pool);
  current_uniform_pool_index += 1;
}

AllocatedBuffer* SKVkFrameBufferData::ObtainTransformBuffer() {
  current_transform_buffer_index++;
  if (current_transform_buffer_index < transform_buffer_.size()) {
    return transform_buffer_[current_transform_buffer_index];
  }

  transform_buffer_.emplace_back(
      allocator_->AllocateUniformBuffer(sizeof(glm::mat4)));

  return transform_buffer_.back();
}

AllocatedBuffer* SKVkFrameBufferData::ObtainCommonSetBuffer() {
  common_set_buffer_index_++;
  if (common_set_buffer_index_ < common_set_buffer_.size()) {
    return common_set_buffer_[common_set_buffer_index_];
  }

  common_set_buffer_.emplace_back(
      allocator_->AllocateUniformBuffer(sizeof(CommonFragmentSet)));

  return common_set_buffer_.back();
}

AllocatedBuffer* SKVkFrameBufferData::ObtainUniformColorBuffer() {
  color_buffer_index++;
  if (color_buffer_index < uniform_color_buffer_.size()) {
    return uniform_color_buffer_[color_buffer_index];
  }

  uniform_color_buffer_.emplace_back(
      allocator_->AllocateUniformBuffer(sizeof(ColorInfoSet)));

  return uniform_color_buffer_.back();
}

AllocatedBuffer* SKVkFrameBufferData::ObtainGradientBuffer() {
  gradient_info_index++;
  if (gradient_info_index < gradient_info_buffer_.size()) {
    return gradient_info_buffer_[gradient_info_index];
  }

  gradient_info_buffer_.emplace_back(
      allocator_->AllocateUniformBuffer(sizeof(GradientInfo)));

  return gradient_info_buffer_.back();
}

VkDescriptorSet SKVkFrameBufferData::ObtainUniformBufferSet(
    GPUVkContext* ctx, VkDescriptorSetLayout layout) {
  VkDescriptorSet ret = VK_NULL_HANDLE;

  auto allocate_info = VKUtils::DescriptorSetAllocateInfo(
      uniform_buffer_pool_[current_uniform_pool_index], &layout, 1);

  auto result =
      VK_CALL(vkAllocateDescriptorSets, ctx->GetDevice(), &allocate_info, &ret);

  if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
    AppendUniformBufferPool(ctx);

    allocate_info.descriptorPool =
        uniform_buffer_pool_[current_uniform_pool_index];

    result = VK_CALL(vkAllocateDescriptorSets, ctx->GetDevice(), &allocate_info,
                     &ret);
  }

  if (result != VK_SUCCESS) {
    LOG_ERROR("Failed to allocate descriptor set for transform uniform buffer");

    return VK_NULL_HANDLE;
  }

  return ret;
}

}  // namespace skity