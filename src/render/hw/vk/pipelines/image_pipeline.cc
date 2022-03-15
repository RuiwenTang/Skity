#include "src/render/hw/vk/pipelines/image_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStaticImagePipeline(VKInterface* vk_interface,
                                              GPUVkContext* ctx, bool use_gs) {
  return PipelineBuilder<StaticImagePipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_image_color_frag_spv,
      vk_image_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStaticImagePipeline(VKInterface* vk_interface,
                                              GPUVkContext* ctx, bool use_gs,
                                              VkRenderPass render_pass) {
  return PipelineBuilder<StaticImagePipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_image_color_frag_spv,
      vk_image_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
      render_pass,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilImagePipeline(VKInterface* vk_interface,
                                               GPUVkContext* ctx, bool use_gs) {
  return PipelineBuilder<StencilDiscardImagePipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_image_color_frag_spv,
      vk_image_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilImagePipeline(VKInterface* vk_interface,
                                               GPUVkContext* ctx, bool use_gs,
                                               VkRenderPass render_pass) {
  return PipelineBuilder<StencilDiscardImagePipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_image_color_frag_spv,
      vk_image_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
      render_pass,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilClipImagePipeline(VKInterface* vk_interface,
                                                   GPUVkContext* ctx,
                                                   bool use_gs) {
  return PipelineBuilder<StencilClipImagePipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_image_color_frag_spv,
      vk_image_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilKeepImagePipeline(VKInterface* vk_interface,
                                                   GPUVkContext* ctx,
                                                   bool use_gs) {
  return PipelineBuilder<StencilKeepImagePipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_image_color_frag_spv,
      vk_image_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

void StaticImagePipeline::UploadGradientInfo(GradientInfo const& info,
                                             GPUVkContext* ctx,
                                             SKVkFrameBufferData* frame_buffer,
                                             VKMemoryAllocator* allocator) {
  bounds_info_ = info.bounds;
}

void StaticImagePipeline::UploadImageTexture(VKTexture* texture,
                                             GPUVkContext* ctx,
                                             SKVkFrameBufferData* frame_buffer,
                                             VKMemoryAllocator* allocator) {
  // reuse common set info struct
  // since image bounds is the same size as common set info
  auto buffer = frame_buffer->ObtainCommonSetBuffer();

  allocator->UploadBuffer(buffer, (void*)&bounds_info_, sizeof(glm::vec4));

  auto image_info = VKUtils::DescriptorImageInfo(texture->GetSampler(),
                                                 texture->GetImageView(),
                                                 texture->GetImageLayout());
  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, GetColorSetLayout());

  VkDescriptorBufferInfo buffer_info{buffer->GetBuffer(), 0, sizeof(glm::vec4)};

  // create VkWriteDescriptorSet to update set
  std::array<VkWriteDescriptorSet, 2> write_sets{};
  write_sets[0] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &buffer_info);
  write_sets[1] = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
      &image_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), write_sets.size(),
          write_sets.data(), 0, VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkDescriptorSetLayout StaticImagePipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  std::array<VkDescriptorSetLayoutBinding, 2> binding = {};
  // binding 0 is
  binding[0] = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
  binding[1] = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
      1);

  auto create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(binding.data(), binding.size());

  return VKUtils::CreateDescriptorSetLayout(GetInterface(), ctx->GetDevice(),
                                            create_info);
}

VkPipelineDepthStencilStateCreateInfo
StencilDiscardImagePipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilDiscardInfo();
}

void StencilClipImagePipeline::UpdateStencilInfo(uint32_t reference,
                                                 uint32_t compare_mask,
                                                 uint32_t write_mask,
                                                 GPUVkContext* ctx) {
  VK_CALL(vkCmdSetStencilReference, GetBindCMD(),
          VK_STENCIL_FACE_FRONT_AND_BACK, reference);
  VK_CALL(vkCmdSetStencilCompareMask, GetBindCMD(),
          VK_STENCIL_FACE_FRONT_AND_BACK, compare_mask);
  VK_CALL(vkCmdSetStencilWriteMask, GetBindCMD(),
          VK_STENCIL_FACE_FRONT_AND_BACK, write_mask);
}

std::vector<VkDynamicState> StencilClipImagePipeline::GetDynamicStates() {
  auto dynamic_states = StaticImagePipeline::GetDynamicStates();

  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);

  return dynamic_states;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipImagePipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilLessDiscardInfo();
}

VkPipelineDepthStencilStateCreateInfo
StencilKeepImagePipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilKeepInfo();
}

}  // namespace skity