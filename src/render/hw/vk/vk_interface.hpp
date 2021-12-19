#ifndef SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP

#include <vulkan/vulkan.h>

namespace skity {

#define VK_CALL(name, ...) VKInterface::GlobalInterface()->f##name(__VA_ARGS__)

struct VKInterface {
  static VKInterface* GlobalInterface();
  static void InitGlobalInterface(VkDevice device,
                                  PFN_vkGetDeviceProcAddr proc_loader);

  PFN_vkAllocateCommandBuffers fvkAllocateCommandBuffers = {};
  PFN_vkAllocateDescriptorSets fvkAllocateDescriptorSets = {};
  PFN_vkBeginCommandBuffer fvkBeginCommandBuffer = {};
  PFN_vkCmdBindDescriptorSets fvkCmdBindDescriptorSets = {};
  PFN_vkCmdBindIndexBuffer fvkCmdBindIndexBuffer = {};
  PFN_vkCmdBindPipeline fvkCmdBindPipeline = {};
  PFN_vkCmdBindVertexBuffers fvkCmdBindVertexBuffers = {};
  PFN_vkCmdCopyBufferToImage fvkCmdCopyBufferToImage = {};
  PFN_vkCmdDrawIndexed fvkCmdDrawIndexed = {};
  PFN_vkCmdPipelineBarrier fvkCmdPipelineBarrier = {};
  PFN_vkCmdPushConstants fvkCmdPushConstants = {};
  PFN_vkCmdSetScissor fvkCmdSetScissor = {};
  PFN_vkCmdSetStencilReference fvkCmdSetStencilReference = {};
  PFN_vkCmdSetViewport fvkCmdSetViewport = {};
  PFN_vkCreateCommandPool fvkCreateCommandPool = {};
  PFN_vkCreateDescriptorPool fvkCreateDescriptorPool = {};
  PFN_vkCreateDescriptorSetLayout fvkCreateDescriptorSetLayout = {};
  PFN_vkCreateFence fvkCreateFence = {};
  PFN_vkCreateGraphicsPipelines fvkCreateGraphicsPipelines = {};
  PFN_vkCreatePipelineLayout fvkCreatePipelineLayout = {};
  PFN_vkCreateShaderModule fvkCreateShaderModule = {};
  PFN_vkDestroyCommandPool fvkDestroyCommandPool = {};
  PFN_vkDestroyDescriptorPool fvkDestroyDescriptorPool = {};
  PFN_vkDestroyDescriptorSetLayout fvkDestroyDescriptorSetLayout = {};
  PFN_vkDestroyFence fvkDestroyFence = {};
  PFN_vkDestroyPipeline fvkDestroyPipeline = {};
  PFN_vkDestroyPipelineLayout fvkDestroyPipelineLayout = {};
  PFN_vkDestroyShaderModule fvkDestroyShaderModule = {};
  PFN_vkEndCommandBuffer fvkEndCommandBuffer = {};
  PFN_vkQueueSubmit fvkQueueSubmit = {};
  PFN_vkResetCommandPool fvkResetCommandPool = {};
  PFN_vkResetDescriptorPool fvkResetDescriptorPool = {};
  PFN_vkResetFences fvkResetFences = {};
  PFN_vkUpdateDescriptorSets fvkUpdateDescriptorSets = {};
  PFN_vkWaitForFences fvkWaitForFences = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP