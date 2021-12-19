#include "src/render/hw/vk/vk_interface.hpp"

namespace skity {

VKInterface* g_vk_interface = nullptr;

#define GET_PROC(F)      \
  g_vk_interface->f##F = \
      (decltype(g_vk_interface->f##F))proc_loader(device, "" #F)

VKInterface* VKInterface::GlobalInterface() { return g_vk_interface; }

void VKInterface::InitGlobalInterface(VkDevice device,
                                      PFN_vkGetDeviceProcAddr proc_loader) {
  g_vk_interface = new VKInterface;

  GET_PROC(vkAllocateCommandBuffers);
  GET_PROC(vkAllocateDescriptorSets);
  GET_PROC(vkBeginCommandBuffer);
  GET_PROC(vkCmdBindDescriptorSets);
  GET_PROC(vkCmdBindIndexBuffer);
  GET_PROC(vkCmdBindPipeline);
  GET_PROC(vkCmdBindVertexBuffers);
  GET_PROC(vkCmdCopyBufferToImage);
  GET_PROC(vkCmdDrawIndexed);
  GET_PROC(vkCmdPipelineBarrier);
  GET_PROC(vkCmdPushConstants);
  GET_PROC(vkCmdSetScissor);
  GET_PROC(vkCmdSetStencilReference);
  GET_PROC(vkCmdSetViewport);
  GET_PROC(vkCreateCommandPool);
  GET_PROC(vkCreateDescriptorPool);
  GET_PROC(vkCreateDescriptorSetLayout);
  GET_PROC(vkCreateFence);
  GET_PROC(vkCreateGraphicsPipelines);
  GET_PROC(vkCreatePipelineLayout);
  GET_PROC(vkCreateShaderModule);
  GET_PROC(vkDestroyCommandPool);
  GET_PROC(vkDestroyDescriptorPool);
  GET_PROC(vkDestroyDescriptorSetLayout);
  GET_PROC(vkDestroyFence);
  GET_PROC(vkDestroyPipeline);
  GET_PROC(vkDestroyPipelineLayout);
  GET_PROC(vkDestroyShaderModule);
  GET_PROC(vkEndCommandBuffer);
  GET_PROC(vkQueueSubmit);
  GET_PROC(vkResetCommandPool);
  GET_PROC(vkResetDescriptorPool);
  GET_PROC(vkResetFences);
  GET_PROC(vkUpdateDescriptorSets);
  GET_PROC(vkWaitForFences);
}

}  // namespace skity