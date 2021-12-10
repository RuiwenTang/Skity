#ifndef SKITY_SRC_RENDER_HW_VK_VK_UTILS_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_UTILS_HPP

#include <vulkan/vulkan.h>

#include <vector>

namespace skity {

class VKUtils final {
 public:
  VKUtils() = delete;
  ~VKUtils() = delete;

  static VkPushConstantRange PushConstantRange(VkShaderStageFlags flags,
                                               uint32_t size, uint32_t offset);

  static VkShaderModule CreateShader(VkDevice, const char* data,
                                     size_t data_size);

  static VkPipelineVertexInputStateCreateInfo
  PipelineVertexInputStateCreateInfo();

  static VkPipelineInputAssemblyStateCreateInfo
  PipelineInputAssemblyStateCreateInfo(
      VkPrimitiveTopology topology,
      VkPipelineInputAssemblyStateCreateFlags flags,
      VkBool32 primitive_restart_enable);

  static VkPipelineRasterizationStateCreateInfo
  PipelineRasterizationStateCreateInfo(
      VkPolygonMode polygon_mode, VkCullModeFlags cull_mode,
      VkFrontFace front_face,
      VkPipelineRasterizationStateCreateFlags flags = 0);

  static VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(
      uint32_t attachment_count,
      VkPipelineColorBlendAttachmentState* p_attachments);

  static VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(
      uint32_t viewport_count, uint32_t scissor_count,
      VkPipelineViewportStateCreateFlags flags = 0);

  static VkPipelineMultisampleStateCreateInfo
  PipelineMultisampleStateCreateInfo(
      VkSampleCountFlagBits rasterization_samples,
      VkPipelineMultisampleStateCreateFlags flags = 0);

  static VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(
      std::vector<VkDynamicState> const& dynamic_states,
      VkPipelineDynamicStateCreateFlags flags = 0);

  static VkGraphicsPipelineCreateInfo PipelineCreateInfo(
      VkPipelineLayout layout, VkRenderPass render_pass,
      VkPipelineCreateFlags flags = 0);

  static VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(
      VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding,
      uint32_t descriptor_count = 1);

  static VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
      const VkDescriptorSetLayoutBinding* p_bindings, uint32_t binding_count);

  static VkDescriptorSetLayout CreateDescriptorSetLayout(
      VkDevice device, VkDescriptorSetLayoutCreateInfo const& create_info);

  static VkPipelineDepthStencilStateCreateInfo
  PipelineDepthStencilStateCreateInfo(VkBool32 depth_test_enable,
                                      VkBool32 depth_write_enable,
                                      VkCompareOp depth_compare_op);
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_UTILS_HPP