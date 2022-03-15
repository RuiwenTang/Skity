#ifndef SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP

#include <vulkan/vulkan.h>

namespace skity {

#define VK_CALL(name, ...) GetInterface()->f##name(__VA_ARGS__)
#define VK_CALL_I(name, ...) vk_interface->f##name(__VA_ARGS__)

struct VKInterface {
  static VKInterface* InitInterface(VkDevice device,
                                    PFN_vkGetDeviceProcAddr proc_loader);

  PFN_vkAllocateCommandBuffers fvkAllocateCommandBuffers = {};
  PFN_vkAllocateDescriptorSets fvkAllocateDescriptorSets = {};
  PFN_vkBeginCommandBuffer fvkBeginCommandBuffer = {};
  PFN_vkCmdBeginRenderPass fvkCmdBeginRenderPass = {};
  PFN_vkCmdBindDescriptorSets fvkCmdBindDescriptorSets = {};
  PFN_vkCmdBindIndexBuffer fvkCmdBindIndexBuffer = {};
  PFN_vkCmdBindPipeline fvkCmdBindPipeline = {};
  PFN_vkCmdBindVertexBuffers fvkCmdBindVertexBuffers = {};
  PFN_vkCmdCopyBufferToImage fvkCmdCopyBufferToImage = {};
  PFN_vkCmdDispatch fvkCmdDispatch = {};
  PFN_vkCmdDrawIndexed fvkCmdDrawIndexed = {};
  PFN_vkCmdEndRenderPass fvkCmdEndRenderPass = {};
  PFN_vkCmdPipelineBarrier fvkCmdPipelineBarrier = {};
  PFN_vkCmdPushConstants fvkCmdPushConstants = {};
  PFN_vkCmdSetScissor fvkCmdSetScissor = {};
  PFN_vkCmdSetStencilReference fvkCmdSetStencilReference = {};
  PFN_vkCmdSetStencilCompareMask fvkCmdSetStencilCompareMask = {};
  PFN_vkCmdSetStencilWriteMask fvkCmdSetStencilWriteMask = {};
  PFN_vkCmdSetViewport fvkCmdSetViewport = {};
  PFN_vkCreateCommandPool fvkCreateCommandPool = {};
  PFN_vkCreateComputePipelines fvkCreateComputePipelines = {};
  PFN_vkCreateDescriptorPool fvkCreateDescriptorPool = {};
  PFN_vkCreateDescriptorSetLayout fvkCreateDescriptorSetLayout = {};
  PFN_vkCreateFence fvkCreateFence = {};
  PFN_vkCreateFramebuffer fvkCreateFramebuffer = {};
  PFN_vkCreateGraphicsPipelines fvkCreateGraphicsPipelines = {};
  PFN_vkCreateImageView fvkCreateImageView = {};
  PFN_vkCreatePipelineLayout fvkCreatePipelineLayout = {};
  PFN_vkCreateRenderPass fvkCreateRenderPass = {};
  PFN_vkCreateSampler fvkCreateSampler = {};
  PFN_vkCreateShaderModule fvkCreateShaderModule = {};
  PFN_vkDestroyCommandPool fvkDestroyCommandPool = {};
  PFN_vkDestroyDescriptorPool fvkDestroyDescriptorPool = {};
  PFN_vkDestroyDescriptorSetLayout fvkDestroyDescriptorSetLayout = {};
  PFN_vkDestroyFence fvkDestroyFence = {};
  PFN_vkDestroyFramebuffer fvkDestroyFramebuffer = {};
  PFN_vkDestroyImageView fvkDestroyImageView = {};
  PFN_vkDestroyPipeline fvkDestroyPipeline = {};
  PFN_vkDestroyPipelineLayout fvkDestroyPipelineLayout = {};
  PFN_vkDestroyRenderPass fvkDestroyRenderPass = {};
  PFN_vkDestroySampler fvkDestroySampler = {};
  PFN_vkDestroyShaderModule fvkDestroyShaderModule = {};
  PFN_vkEndCommandBuffer fvkEndCommandBuffer = {};
  PFN_vkGetPhysicalDeviceFeatures fvkGetPhysicalDeviceFeatures = {};
  PFN_vkQueueSubmit fvkQueueSubmit = {};
  PFN_vkQueueWaitIdle fvkQueueWaitIdle = {};
  PFN_vkResetCommandPool fvkResetCommandPool = {};
  PFN_vkResetDescriptorPool fvkResetDescriptorPool = {};
  PFN_vkResetFences fvkResetFences = {};
  PFN_vkUpdateDescriptorSets fvkUpdateDescriptorSets = {};
  PFN_vkWaitForFences fvkWaitForFences = {};
};

class VkInterfaceClient {
 public:
  VkInterfaceClient() = default;
  VkInterfaceClient(VKInterface* interface) : interface_(interface) {}
  ~VkInterfaceClient() = default;

  VKInterface* GetInterface() const { return interface_; }

  void SetInterface(VKInterface* interface) { interface_ = interface; }

 private:
  VKInterface* interface_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP