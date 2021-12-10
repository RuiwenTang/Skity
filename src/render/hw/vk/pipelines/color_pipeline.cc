#include "src/render/hw/vk/pipelines/color_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

std::unique_ptr<VKPipelineWrapper> VKPipelineWrapper::CreateStaticColorPipeline(
    GPUVkContext* ctx) {
  auto static_color_pipeline =
      std::make_unique<StaticColorPipeline>(sizeof(GlobalPushConst));

  auto vertex =
      VKUtils::CreateShader(ctx->GetDevice(), (const char*)vk_common_vert_spv,
                            vk_common_vert_spv_size);

  auto fragment = VKUtils::CreateShader(ctx->GetDevice(),
                                        (const char*)vk_uniform_color_frag_spv,
                                        vk_uniform_color_frag_spv_size);
  static_color_pipeline->Init(ctx, vertex, fragment);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vertex, nullptr);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fragment, nullptr);

  return static_color_pipeline;
}

static VkDescriptorSetLayout create_color_descriptor_set_layout(
    GPUVkContext* ctx) {
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

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

std::vector<VkDescriptorSetLayout>
StaticColorPipeline::GenearteDescriptorSetLayout(GPUVkContext* ctx) {
  auto layouts = VKPipelineWrapper::GenearteDescriptorSetLayout(ctx);

  layouts.emplace_back(create_color_descriptor_set_layout(ctx));

  return layouts;
}

}  // namespace skity