#include "src/render/hw/vk/pipelines/gradient_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStaticGradientPipeline(GPUVkContext* ctx) {
  auto pipeline =
      std::make_unique<StaticGradientPipeline>(sizeof(GlobalPushConst));

  auto vertex =
      VKUtils::CreateShader(ctx->GetDevice(), (const char*)vk_common_vert_spv,
                            vk_common_vert_spv_size);

  auto fragment = VKUtils::CreateShader(ctx->GetDevice(),
                                        (const char*)vk_gradient_color_frag_spv,
                                        vk_gradient_color_frag_spv_size);

  pipeline->Init(ctx, vertex, fragment);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vertex, nullptr);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fragment, nullptr);

  return pipeline;
}

void StaticGradientPipeline::UploadGradientInfo(GradientInfo const& info,
                                                GPUVkContext* ctx,
                                                VKFrameBuffer* frame_buffer,
                                                VKMemoryAllocator* allocator) {
  auto buffer = frame_buffer->ObtainGradientBuffer();
  allocator->UploadBuffer(buffer, (void*)&info, sizeof(GradientInfo));

  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, GetColorSetLayout());

  VkDescriptorBufferInfo binding_info{
      buffer->GetBuffer(),
      0,
      sizeof(GradientInfo),
  };
  auto write_set = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &binding_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), 1, &write_set, 0,
          VK_NULL_HANDLE);
  VK_CALL(vkCmdBindDescriptorSets, ctx->GetCurrentCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkDescriptorSetLayout StaticGradientPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  auto binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto create_info = VKUtils::DescriptorSetLayoutCreateInfo(&binding, 1);

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

}  // namespace skity