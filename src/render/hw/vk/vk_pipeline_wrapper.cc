#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

void VKPipelineWrapper::Init(GPUVkContext *ctx, VkShaderModule vertex,
                             VkShaderModule fragment) {
  // pipeline layout
  VkPushConstantRange push_const_range = VKUtils::PushConstantRange(
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      push_const_size_, 0);
}

void VKPipelineWrapper::Bind(VkCommandBuffer cmd) {
  VK_CALL(vkCmdBindPipeline, cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}

}  // namespace skity