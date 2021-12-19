#include "src/render/hw/vk/vk_utils.hpp"

#ifndef SKITY_RELEASE
#include <cassert>
#endif

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"

namespace skity {

VkPushConstantRange VKUtils::PushConstantRange(VkShaderStageFlags flags,
                                               uint32_t size, uint32_t offset) {
  VkPushConstantRange ret = {};
  ret.stageFlags = flags;
  ret.offset = offset;
  ret.size = size;

  return ret;
}

VkShaderModule VKUtils::CreateShader(VkDevice device, const char *data,
                                     size_t data_size) {
  VkShaderModule shader_module = {};

  VkShaderModuleCreateInfo create_info{
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

  create_info.codeSize = data_size;
  create_info.pCode = (uint32_t *)data;

  if (VK_CALL(vkCreateShaderModule, device, &create_info, nullptr,
              &shader_module) != VK_SUCCESS) {
    LOG_ERROR("Failed to create shader module!!");
#ifndef SKITY_RELEASE
    assert(false);
#endif
    return nullptr;
  }

  return shader_module;
}

VkPipelineVertexInputStateCreateInfo
VKUtils::PipelineVertexInputStateCreateInfo() {
  VkPipelineVertexInputStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  return create_info;
}

VkPipelineInputAssemblyStateCreateInfo
VKUtils::PipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags,
    VkBool32 primitive_restart_enable) {
  VkPipelineInputAssemblyStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

  create_info.topology = topology;
  create_info.flags = flags;
  create_info.primitiveRestartEnable = primitive_restart_enable;

  return create_info;
}

VkPipelineRasterizationStateCreateInfo
VKUtils::PipelineRasterizationStateCreateInfo(
    VkPolygonMode polygon_mode, VkCullModeFlags cull_mode,
    VkFrontFace front_face, VkPipelineRasterizationStateCreateFlags flags) {
  VkPipelineRasterizationStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

  create_info.polygonMode = polygon_mode;
  create_info.cullMode = cull_mode;
  create_info.frontFace = front_face;
  create_info.flags = flags;
  create_info.depthClampEnable = VK_FALSE;
  create_info.lineWidth = 1.f;

  return create_info;
}

VkPipelineColorBlendStateCreateInfo VKUtils::PipelineColorBlendStateCreateInfo(
    uint32_t attachment_count,
    VkPipelineColorBlendAttachmentState *p_attachments) {
  VkPipelineColorBlendStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

  create_info.attachmentCount = attachment_count;
  create_info.pAttachments = p_attachments;

  return create_info;
}

VkPipelineViewportStateCreateInfo VKUtils::PipelineViewportStateCreateInfo(
    uint32_t viewport_count, uint32_t scissor_count,
    VkPipelineViewportStateCreateFlags flags) {
  VkPipelineViewportStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

  create_info.viewportCount = 1;
  create_info.scissorCount = 1;
  create_info.flags = flags;
  return create_info;
}

VkPipelineMultisampleStateCreateInfo
VKUtils::PipelineMultisampleStateCreateInfo(
    VkSampleCountFlagBits rasterization_samples,
    VkPipelineMultisampleStateCreateFlags flags) {
  VkPipelineMultisampleStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

  create_info.rasterizationSamples = rasterization_samples;
  create_info.flags = flags;
  return create_info;
}

VkPipelineDynamicStateCreateInfo VKUtils::PipelineDynamicStateCreateInfo(
    const std::vector<VkDynamicState> &dynamic_states,
    VkPipelineDynamicStateCreateFlags flags) {
  VkPipelineDynamicStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

  create_info.pDynamicStates = dynamic_states.data();
  create_info.dynamicStateCount = dynamic_states.size();
  create_info.flags = flags;
  return create_info;
}

VkGraphicsPipelineCreateInfo VKUtils::PipelineCreateInfo(
    VkPipelineLayout layout, VkRenderPass render_pass,
    VkPipelineCreateFlags flags) {
  VkGraphicsPipelineCreateInfo create_info{
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

  create_info.layout = layout;
  create_info.renderPass = render_pass;
  create_info.flags = flags;
  create_info.basePipelineIndex = -1;
  create_info.basePipelineHandle = VK_NULL_HANDLE;
  return create_info;
}

VkDescriptorSetLayoutBinding VKUtils::DescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding,
    uint32_t descriptor_count) {
  VkDescriptorSetLayoutBinding set_layout_binding{};

  set_layout_binding.descriptorType = type;
  set_layout_binding.stageFlags = stage_flags;
  set_layout_binding.binding = binding;
  set_layout_binding.descriptorCount = descriptor_count;

  return set_layout_binding;
}

VkDescriptorSetLayoutCreateInfo VKUtils::DescriptorSetLayoutCreateInfo(
    const VkDescriptorSetLayoutBinding *p_bindings, uint32_t binding_count) {
  VkDescriptorSetLayoutCreateInfo create_info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

  create_info.pBindings = p_bindings;
  create_info.bindingCount = binding_count;

  return create_info;
}

VkDescriptorSetLayout VKUtils::CreateDescriptorSetLayout(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo &create_info) {
  VkDescriptorSetLayout ret = VK_NULL_HANDLE;

  if (VK_CALL(vkCreateDescriptorSetLayout, device, &create_info, nullptr,
              &ret) != VK_SUCCESS) {
    LOG_ERROR("Failed create DescriptorSet layout with {} bindings",
              create_info.bindingCount);
  }

  return ret;
}

VkPipelineDepthStencilStateCreateInfo
VKUtils::PipelineDepthStencilStateCreateInfo(VkBool32 depth_test_enable,
                                             VkBool32 depth_write_enable,
                                             VkCompareOp depth_compare_op) {
  VkPipelineDepthStencilStateCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  create_info.depthTestEnable = depth_test_enable;
  create_info.depthWriteEnable = depth_write_enable;
  create_info.depthCompareOp = depth_compare_op;

  return create_info;
}

VkDescriptorPoolCreateInfo VKUtils::DescriptorPoolCreateInfo(
    uint32_t pool_size_count, VkDescriptorPoolSize *p_pool_size,
    uint32_t max_sets) {
  VkDescriptorPoolCreateInfo create_info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

  create_info.poolSizeCount = pool_size_count;
  create_info.pPoolSizes = p_pool_size;
  create_info.maxSets = max_sets;

  return create_info;
}

VkDescriptorSetAllocateInfo VKUtils::DescriptorSetAllocateInfo(
    VkDescriptorPool pool, VkDescriptorSetLayout *p_set_layouts,
    uint32_t descriptor_set_count) {
  VkDescriptorSetAllocateInfo allocate_info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};

  allocate_info.descriptorPool = pool;
  allocate_info.pSetLayouts = p_set_layouts;
  allocate_info.descriptorSetCount = descriptor_set_count;

  return allocate_info;
}

VkWriteDescriptorSet VKUtils::WriteDescriptorSet(
    VkDescriptorSet dst_set, VkDescriptorType type, uint32_t binding,
    VkDescriptorBufferInfo *buffer_info, uint32_t descriptor_count) {
  VkWriteDescriptorSet write_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

  write_set.dstSet = dst_set;
  write_set.descriptorType = type;
  write_set.dstBinding = binding;
  write_set.pBufferInfo = buffer_info;
  write_set.descriptorCount = descriptor_count;

  return write_set;
}

VkWriteDescriptorSet VKUtils::WriteDescriptorSet(
    VkDescriptorSet dst_set, VkDescriptorType type, uint32_t binding,
    VkDescriptorImageInfo *image_info, uint32_t descriptor_count) {
  VkWriteDescriptorSet write_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

  write_set.dstSet = dst_set;
  write_set.descriptorType = type;
  write_set.dstBinding = binding;
  write_set.pImageInfo = image_info;
  write_set.descriptorCount = descriptor_count;

  return write_set;
}

VkDescriptorImageInfo VKUtils::DescriptorImageInfo(VkSampler sampler,
                                                   VkImageView image_view,
                                                   VkImageLayout layout) {
  VkDescriptorImageInfo info{};

  info.sampler = sampler;
  info.imageView = image_view;
  info.imageLayout = layout;

  return info;
}

VkSamplerCreateInfo VKUtils::SamplerCreateInfo() {
  VkSamplerCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  create_info.maxAnisotropy = 1.0f;

  return create_info;
}

VkImageCreateInfo VKUtils::ImageCreateInfo(VkFormat format,
                                           VkImageUsageFlags flags,
                                           VkExtent3D extent) {
  VkImageCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

  create_info.imageType = VK_IMAGE_TYPE_2D;
  create_info.extent = extent;
  create_info.mipLevels = 1;
  create_info.arrayLayers = 1;
  create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  create_info.usage = flags;
  create_info.format = format;

  return create_info;
}

VkImageViewCreateInfo VKUtils::ImageViewCreateInfo(
    VkFormat format, VkImage image, VkImageSubresourceRange const &range) {
  VkImageViewCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.image = image;
  create_info.subresourceRange = range;

  return create_info;
}

void VKUtils::SetImageLayout(VkCommandBuffer cmd, VkImage image,
                             VkImageAspectFlags aspect_mask,
                             VkImageLayout old_layout, VkImageLayout new_layout,
                             VkImageSubresourceRange range,
                             VkPipelineStageFlags src_stage,
                             VkPipelineStageFlags dst_stage) {
  VkImageMemoryBarrier image_memory_barrier{
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  image_memory_barrier.oldLayout = old_layout;
  image_memory_barrier.newLayout = new_layout;
  image_memory_barrier.image = image;
  image_memory_barrier.subresourceRange = range;

  // source layouts(old)
  // source access mask controls that have to be finished on the old layout
  // before it will be transitioned to the new layout
  switch (old_layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      // Image layout is undefined(or does not matter)
      // Only valid as init layout
      // No flags required, listed only for completeness
      image_memory_barrier.srcAccessMask = 0;
      break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      // Image is preinitialized
      // Only valid as initial layout for linear images, preserves memory
      // contents Make sure host writes have been finished
      image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      // Image is a color attachment
      // Make sure any writes to the color buffer have been finished
      image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      // Image is depth/stencil attachment
      // Make sure any writes to the depth/stencil buffer have been finished
      image_memory_barrier.srcAccessMask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      // Image is a transfer source
      // Make sure any reads from the images have been finished
      image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      // Image is a transfer destination
      // Make sure any writes to the image have been finished
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      // Image is read by a shader
      // Make sure any shader reads from the image have been finished
      image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default:
      LOG_WARN(
          "Other source layouts aren't handled, and may cause error in vulkan");
      break;
  }

  // Target layouts(new)
  // Destination access mask controls the dependency for the new image layout
  switch (new_layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      // Image will be used as a transfer destination
      // Make sure any writes to the image have been finished
      image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      // Image will be used as a transfer source
      // Make sure any reads from the image have been finished
      image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      // Image will be used as a color attachment
      // Make sure any writes to the color buffer have been finished
      image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      // Image layout will be used as a depth/stencil attachment
      // Make sure any writes to depth/stencil buffer have been finished
      image_memory_barrier.dstAccessMask =
          image_memory_barrier.dstAccessMask |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      // Image will be read in a shader (sampler, input attachment)
      // Make sure any writes to the image have been finished
      if (image_memory_barrier.srcAccessMask == 0) {
        image_memory_barrier.srcAccessMask =
            VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      }
      image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default:
      LOG_WARN("Other source layouts aren't handled (yet)");
      break;
  }

  // Put barrier inside setup command buffer
  VK_CALL(vkCmdPipelineBarrier, cmd, src_stage, dst_stage, 0, 0, nullptr, 0,
          nullptr, 1, &image_memory_barrier);
}

}  // namespace skity
