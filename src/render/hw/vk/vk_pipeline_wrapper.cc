#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

void VKPipelineWrapper::Init(GPUVkContext* ctx, VkShaderModule vertex,
                             VkShaderModule fragment) {
  InitDescriptorSetLayout(ctx);
  InitPipelineLayout(ctx);
}

void VKPipelineWrapper::Bind(VkCommandBuffer cmd) {
  VK_CALL(vkCmdBindPipeline, cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}

void VKPipelineWrapper::InitDescriptorSetLayout(GPUVkContext* ctx) {
  descriptor_set_layout_ = GenearteDescriptorSetLayout(ctx);
}

void VKPipelineWrapper::InitPipelineLayout(GPUVkContext* ctx) {
  VkPushConstantRange push_const_range = VKUtils::PushConstantRange(
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      push_const_size_, 0);

  VkPipelineLayoutCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  create_info.setLayoutCount = descriptor_set_layout_.size();
  create_info.pSetLayouts = descriptor_set_layout_.data();
  create_info.pushConstantRangeCount = 1;
  create_info.pPushConstantRanges = &push_const_range;

  if (VK_CALL(vkCreatePipelineLayout, ctx->GetDevice(), &create_info, nullptr,
              &pipeline_layout_) != VK_SUCCESS) {
    LOG_ERROR("Failed to create pipeline layout for {} descriptor sets",
              descriptor_set_layout_.size());
  }
}

}  // namespace skity