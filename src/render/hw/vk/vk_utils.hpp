#ifndef SKITY_SRC_RENDER_HW_VK_VK_UTILS_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_UTILS_HPP

#include <vulkan/vulkan.h>

namespace skity {

class VKUtils final {
 public:
  VKUtils() = delete;
  ~VKUtils() = delete;

  static VkPushConstantRange PushConstantRange(VkShaderStageFlags flags,
                                               uint32_t size, uint32_t offset);

  static VkShaderModule CreateShader(VkDevice, const char* data,
                                     size_t data_size);
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_UTILS_HPP