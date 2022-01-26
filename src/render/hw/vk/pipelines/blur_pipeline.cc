#include "src/render/hw/vk/pipelines/blur_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStaticBlurPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StaticBlurPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_blur_effect_frag_spv,
      vk_blur_effect_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStaticBlurPipeline(GPUVkContext* ctx,
                                             VkRenderPass render_pass) {
  return PipelineBuilder<StaticBlurPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_blur_effect_frag_spv,
      vk_blur_effect_frag_spv_size,
      ctx,
      render_pass}();
}

VkDescriptorSetLayout StaticBlurPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  std::array<VkDescriptorSetLayoutBinding, 3> binding = {};

  binding[0] = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
  binding[1] = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
  binding[2] = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
      2);

  auto create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(binding.data(), binding.size());

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

void StaticBlurPipeline::UploadBlurInfo(glm::ivec4 const& info,
                                        GPUVkContext* ctx,
                                        SKVkFrameBufferData* frame_buffer,
                                        VKMemoryAllocator* allocator) {
  blur_info_ = info;
}

void StaticBlurPipeline::UploadGradientInfo(GradientInfo const& info,
                                            GPUVkContext* ctx,
                                            SKVkFrameBufferData* frame_buffer,
                                            VKMemoryAllocator* allocator) {
  bounds_info_ = info.bounds;
}

void StaticBlurPipeline::UploadImageTexture(VKTexture* texture,
                                            GPUVkContext* ctx,
                                            SKVkFrameBufferData* frame_buffer,
                                            VKMemoryAllocator* allocator) {
  // reuse common set info struct
  // since image bounds is the same size as common set info
  auto bound_buffer = frame_buffer->ObtainCommonSetBuffer();
  auto blur_buffer = frame_buffer->ObtainCommonSetBuffer();

  allocator->UploadBuffer(bound_buffer, (void*)&bounds_info_,
                          sizeof(glm::vec4));

  allocator->UploadBuffer(blur_buffer, (void*)&blur_info_, sizeof(glm::ivec4));

  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, GetColorSetLayout());

  auto image_info = VKUtils::DescriptorImageInfo(texture->GetSampler(),
                                                 texture->GetImageView(),
                                                 texture->GetImageLayout());

  VkDescriptorBufferInfo bound_buffer_info{bound_buffer->GetBuffer(), 0,
                                           sizeof(glm::vec4)};
  VkDescriptorBufferInfo blur_buffer_info{blur_buffer->GetBuffer(), 0,
                                          sizeof(glm::ivec4)};

  std::array<VkWriteDescriptorSet, 3> write_sets{};
  write_sets[0] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bound_buffer_info);

  write_sets[1] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &blur_buffer_info);

  write_sets[2] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2,
      &image_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), write_sets.size(),
          write_sets.data(), 0, VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkDescriptorSetLayout ComputeBlurPipeline::CreateDescriptorSetLayout(
    GPUVkContext* ctx) {
  return nullptr;
}

void ComputeBlurPipeline::OnDispatch(VkCommandBuffer cmd) {}

}  // namespace skity