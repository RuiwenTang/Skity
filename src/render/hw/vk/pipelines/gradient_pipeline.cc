#include "src/render/hw/vk/pipelines/gradient_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStaticGradientPipeline(VKInterface* vk_interface,
                                                 GPUVkContext* ctx,
                                                 bool use_gs) {
  return PipelineBuilder<StaticGradientPipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_gradient_color_frag_spv,
      vk_gradient_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStaticGradientPipeline(VKInterface* vk_interface,
                                                 GPUVkContext* ctx, bool use_gs,
                                                 VkRenderPass render_pass) {
  return PipelineBuilder<StaticGradientPipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_gradient_color_frag_spv,
      vk_gradient_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
      render_pass,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilDiscardGradientPipeline(
    VKInterface* vk_interface, GPUVkContext* ctx, bool use_gs) {
  return PipelineBuilder<StencilDiscardGradientPipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_gradient_color_frag_spv,
      vk_gradient_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilDiscardGradientPipeline(
    VKInterface* vk_interface, GPUVkContext* ctx, bool use_gs,
    VkRenderPass render_pass) {
  return PipelineBuilder<StencilDiscardGradientPipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_gradient_color_frag_spv,
      vk_gradient_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
      render_pass,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilClipGradientPipeline(VKInterface* vk_interface,
                                                      GPUVkContext* ctx,
                                                      bool use_gs) {
  return PipelineBuilder<StencilClipGradientPipeline>{
      vk_interface,
      use_gs ? (const char*)vk_gs_common_vert_spv
             : (const char*)vk_common_vert_spv,
      use_gs ? vk_gs_common_vert_spv_size : vk_common_vert_spv_size,
      (const char*)vk_gradient_color_frag_spv,
      vk_gradient_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

std::unique_ptr<AbsPipelineWrapper>
AbsPipelineWrapper::CreateStencilKeepGradientPipeline(VKInterface* vk_interface,
                                                      GPUVkContext* ctx,
                                                      bool use_gs) {
  return PipelineBuilder<StencilKeepGradientPipeline>{
      vk_interface,
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_gradient_color_frag_spv,
      vk_gradient_color_frag_spv_size,
      use_gs ? (const char*)vk_gs_geometry_geom_spv : nullptr,
      use_gs ? vk_gs_geometry_geom_spv_size : 0,
      ctx,
  }();
}

void StaticGradientPipeline::UploadGradientInfo(
    GradientInfo const& info, GPUVkContext* ctx,
    SKVkFrameBufferData* frame_buffer, VKMemoryAllocator* allocator) {
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
  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkDescriptorSetLayout StaticGradientPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  auto binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto create_info = VKUtils::DescriptorSetLayoutCreateInfo(&binding, 1);

  return VKUtils::CreateDescriptorSetLayout(GetInterface(), ctx->GetDevice(),
                                            create_info);
}

VkPipelineDepthStencilStateCreateInfo
StencilDiscardGradientPipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilDiscardInfo();
}

void StencilClipGradientPipeline::UpdateStencilInfo(uint32_t reference,
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

std::vector<VkDynamicState> StencilClipGradientPipeline::GetDynamicStates() {
  auto dynamic_states = StaticGradientPipeline::GetDynamicStates();

  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);

  return dynamic_states;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipGradientPipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilLessDiscardInfo();
}

VkPipelineDepthStencilStateCreateInfo
StencilKeepGradientPipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilKeepInfo();
}

}  // namespace skity