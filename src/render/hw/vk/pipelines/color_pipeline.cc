#include "src/render/hw/vk/pipelines/color_pipeline.hpp"

#include <array>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

ColorPipeline::ColorPipeline(size_t push_const_size)
    : VKPipelineWrapper(push_const_size) {}

std::vector<VkDescriptorSetLayout> ColorPipeline::GenearteDescriptorSetLayout(
    GPUVkContext* ctx) {
  auto layouts = VKPipelineWrapper::GenearteDescriptorSetLayout(ctx);

  LOG_DEBUG("Color Pipeline create set 1 layout");
  std::array<VkDescriptorSetLayoutBinding, 2> bindings{
      // set 1 binding 0 global alpha
      VKUtils::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          VK_SHADER_STAGE_FRAGMENT_BIT, 0),
      // set 1 binding 1 uniform color
      VKUtils::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          VK_SHADER_STAGE_FRAGMENT_BIT, 1)};

  auto create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(bindings.data(), bindings.size());

  layouts.emplace_back(
      VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info));

  return layouts;
}

}  // namespace skity