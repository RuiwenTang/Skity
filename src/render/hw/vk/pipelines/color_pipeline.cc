#include "src/render/hw/vk/pipelines/color_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

std::unique_ptr<VKPipelineWrapper> VKPipelineWrapper::CreateStaticColorPipeline(
    GPUVkContext* ctx) {
  return PipelineBuilder<StaticColorPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_uniform_color_frag_spv,
      vk_uniform_color_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilColorPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilDiscardColorPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_uniform_color_frag_spv,
      vk_uniform_color_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilClipColorPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilClipColorPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_uniform_color_frag_spv,
      vk_uniform_color_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilKeepColorPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilKeepColorPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_uniform_color_frag_spv,
      vk_uniform_color_frag_spv_size,
      ctx,
  }();
}

VkDescriptorSetLayout StaticColorPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  auto binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto create_info = VKUtils::DescriptorSetLayoutCreateInfo(&binding, 1);

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

void StaticColorPipeline::UploadUniformColor(ColorInfoSet const& info,
                                             GPUVkContext* ctx,
                                             VKFrameBuffer* frame_buffer,
                                             VKMemoryAllocator* allocator) {
  auto buffer = frame_buffer->ObtainUniformColorBuffer();

  allocator->UploadBuffer(buffer, (void*)&info, sizeof(ColorInfoSet));

  // color info is in set 2
  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, GetColorSetLayout());

  VkDescriptorBufferInfo buffer_info{buffer->GetBuffer(), 0, sizeof(info)};

  // create VkWriteDescriptorSet to update set
  auto write_set = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &buffer_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), 1, &write_set, 0,
          VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, ctx->GetCurrentCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkPipelineDepthStencilStateCreateInfo
StencilDiscardColorPipeline::GetDepthStencilStateCreateInfo() {
  return VKPipelineWrapper::StencilDiscardInfo();
}

VkPipelineDepthStencilStateCreateInfo
StencilClipColorPipeline::GetDepthStencilStateCreateInfo() {
  return VKPipelineWrapper::StencilClipDiscardInfo();
}

VkPipelineDepthStencilStateCreateInfo
StencilKeepColorPipeline::GetDepthStencilStateCreateInfo() {
  return VKPipelineWrapper::StencilKeepInfo();
}

}  // namespace skity