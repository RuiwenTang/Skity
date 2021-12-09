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

}  // namespace skity