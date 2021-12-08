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

}  // namespace skity