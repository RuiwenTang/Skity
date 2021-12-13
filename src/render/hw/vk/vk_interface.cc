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

  GET_PROC(vkCmdBindIndexBuffer);
  GET_PROC(vkCmdBindPipeline);
  GET_PROC(vkCmdBindVertexBuffers);
  GET_PROC(vkCmdPushConstants);
  GET_PROC(vkCmdSetScissor);
  GET_PROC(vkCmdSetViewport);
  GET_PROC(vkCreateDescriptorPool);
  GET_PROC(vkCreateDescriptorSetLayout);
  GET_PROC(vkCreateGraphicsPipelines);
  GET_PROC(vkCreatePipelineLayout);
  GET_PROC(vkCreateShaderModule);
  GET_PROC(vkDestroyDescriptorPool);
  GET_PROC(vkDestroyDescriptorSetLayout);
  GET_PROC(vkDestroyPipeline);
  GET_PROC(vkDestroyPipelineLayout);
  GET_PROC(vkDestroyShaderModule);
  GET_PROC(vkResetDescriptorPool);
}

}  // namespace skity