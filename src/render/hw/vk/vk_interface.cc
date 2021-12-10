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

  GET_PROC(vkCmdBindPipeline);
  GET_PROC(vkCreateDescriptorSetLayout);
  GET_PROC(vkCreateGraphicsPipelines);
  GET_PROC(vkCreatePipelineLayout);
  GET_PROC(vkCreateShaderModule);
  GET_PROC(vkDestroyDescriptorSetLayout);
  GET_PROC(vkDestroyPipeline);
  GET_PROC(vkDestroyPipelineLayout);
  GET_PROC(vkDestroyShaderModule);
}

}  // namespace skity