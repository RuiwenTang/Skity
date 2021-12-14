#ifndef SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP

#include <vulkan/vulkan.h>

namespace skity {

#define VK_CALL(name, ...) VKInterface::GlobalInterface()->f##name(__VA_ARGS__)

struct VKInterface {
  static VKInterface* GlobalInterface();
  static void InitGlobalInterface(VkDevice device,
                                  PFN_vkGetDeviceProcAddr proc_loader);

  PFN_vkAllocateDescriptorSets fvkAllocateDescriptorSets = {};
  PFN_vkCmdBindDescriptorSets fvkCmdBindDescriptorSets = {};
  PFN_vkCmdBindIndexBuffer fvkCmdBindIndexBuffer = {};
  PFN_vkCmdBindPipeline fvkCmdBindPipeline = {};
  PFN_vkCmdBindVertexBuffers fvkCmdBindVertexBuffers = {};
  PFN_vkCmdDrawIndexed fvkCmdDrawIndexed = {};
  PFN_vkCmdPushConstants fvkCmdPushConstants = {};
  PFN_vkCmdSetScissor fvkCmdSetScissor = {};
  PFN_vkCmdSetViewport fvkCmdSetViewport = {};
  PFN_vkCreateDescriptorPool fvkCreateDescriptorPool = {};
  PFN_vkCreateDescriptorSetLayout fvkCreateDescriptorSetLayout = {};
  PFN_vkCreateGraphicsPipelines fvkCreateGraphicsPipelines = {};
  PFN_vkCreatePipelineLayout fvkCreatePipelineLayout = {};
  PFN_vkCreateShaderModule fvkCreateShaderModule = {};
  PFN_vkDestroyDescriptorPool fvkDestroyDescriptorPool = {};
  PFN_vkDestroyDescriptorSetLayout fvkDestroyDescriptorSetLayout = {};
  PFN_vkDestroyPipeline fvkDestroyPipeline = {};
  PFN_vkDestroyPipelineLayout fvkDestroyPipelineLayout = {};
  PFN_vkDestroyShaderModule fvkDestroyShaderModule = {};
  PFN_vkResetDescriptorPool fvkResetDescriptorPool = {};
  PFN_vkUpdateDescriptorSets fvkUpdateDescriptorSets = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP