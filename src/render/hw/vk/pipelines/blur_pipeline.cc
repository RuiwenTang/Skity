#include "src/render/hw/vk/pipelines/blur_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_texture.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

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

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateComputeBlurPipeline(GPUVkContext* ctx) {
  auto compute_shader = VKUtils::CreateShader(
      ctx->GetDevice(), (const char*)vk_blur_effect_comp_spv,
      vk_blur_effect_comp_spv_size);

  auto pipeline = std::make_unique<ComputeBlurPipeline>();

  pipeline->Init(ctx, compute_shader, VK_NULL_HANDLE);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), compute_shader, nullptr);
  return pipeline;
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

void ComputeBlurPipeline::UploadBlurInfo(glm::ivec4 const& info,
                                         GPUVkContext* ctx,
                                         SKVkFrameBufferData* frame_buffer,
                                         VKMemoryAllocator* allocator) {
  blur_info_ = info;
}

VkDescriptorSetLayout ComputeBlurPipeline::CreateDescriptorSetLayout(
    GPUVkContext* ctx) {
  auto binding0 = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0);
  auto binding1 = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
  auto binding2 = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 2);

  std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
      binding0,
      binding1,
      binding2,
  };

  auto create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(bindings.data(), bindings.size());

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

void ComputeBlurPipeline::OnDispatch(VkCommandBuffer cmd, GPUVkContext* ctx) {
  ComputeInfo compute_info{};
  compute_info.info.x = CommonInfo().y;
  compute_info.blur_type.x = blur_info_.x;
  compute_info.blur_type.y = OutpuTexture()->GetWidth();
  compute_info.blur_type.z = OutpuTexture()->GetHeight();
  compute_info.bounds = BoundsInfo();

  auto buffer = FrameBufferData()->ObtainComputeInfoBuffer();

  Allocator()->UploadBuffer(buffer, &compute_info, sizeof(ComputeInfo));

  VkDescriptorBufferInfo buffer_info{buffer->GetBuffer(), 0,
                                     sizeof(ComputeInfo)};

  auto input_image_info = VKUtils::DescriptorImageInfo(
      VK_NULL_HANDLE, InputTexture()->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);

  auto output_image_info = VKUtils::DescriptorImageInfo(
      VK_NULL_HANDLE, OutpuTexture()->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);

  auto descriptor_set =
      FrameBufferData()->ObtainUniformBufferSet(ctx, ComputeSetLayout());
  std::array<VkWriteDescriptorSet, 3> write_sets{};

  write_sets[0] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &buffer_info);

  write_sets[1] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &input_image_info);

  write_sets[2] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, &output_image_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), write_sets.size(),
          write_sets.data(), 0, VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
          GetPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);
}

}  // namespace skity