#include "src/render/hw/vk/pipelines/color_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

VkDescriptorSetLayout StaticColorPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  auto binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto create_info = VKUtils::DescriptorSetLayoutCreateInfo(&binding, 1);

  return VKUtils::CreateDescriptorSetLayout(GetInterface(), ctx->GetDevice(),
                                            create_info);
}

void StaticColorPipeline::UploadUniformColor(ColorInfoSet const& info,
                                             GPUVkContext* ctx,
                                             SKVkFrameBufferData* frame_buffer,
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

  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkPipelineDepthStencilStateCreateInfo
StencilDiscardColorPipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilDiscardInfo();
}

void StencilClipColorPipeline::UpdateStencilInfo(uint32_t reference,
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

std::vector<VkDynamicState> StencilClipColorPipeline::GetDynamicStates() {
  auto dynamic_states = StaticColorPipeline::GetDynamicStates();

  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
  dynamic_states.emplace_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);

  return dynamic_states;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipColorPipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilLessDiscardInfo();
}

VkPipelineDepthStencilStateCreateInfo
StencilKeepColorPipeline::GetDepthStencilStateCreateInfo() {
  return RenderPipeline::StencilKeepInfo();
}

std::tuple<const char*, size_t> ColorPipelineFamily::GetFragmentShaderInfo() {
  return {(const char*)vk_uniform_color_frag_spv,
          vk_uniform_color_frag_spv_size};
}

std::unique_ptr<AbsPipelineWrapper> ColorPipelineFamily::CreateStaticPipeline(
    GPUVkContext* ctx) {
  auto pipeline = std::make_unique<StaticColorPipeline>(
      UseGeometryShader(), sizeof(GlobalPushConst));

  pipeline->SetInterface(GetInterface());

  pipeline->Init(ctx, GetVertexShader(), GetFragmentShader(),
                 GetGeometryShader());

  return pipeline;
}

std::unique_ptr<AbsPipelineWrapper>
ColorPipelineFamily::CreateStencilDiscardPipeline(GPUVkContext* ctx) {
  auto pipeline = std::make_unique<StencilDiscardColorPipeline>(
      UseGeometryShader(), sizeof(GlobalPushConst));

  pipeline->SetInterface(GetInterface());

  pipeline->Init(ctx, GetVertexShader(), GetFragmentShader(),
                 GetGeometryShader());

  return pipeline;
}

std::unique_ptr<AbsPipelineWrapper>
ColorPipelineFamily::CreateStencilClipPipeline(GPUVkContext* ctx) {
  auto pipeline = std::make_unique<StencilClipColorPipeline>(
      UseGeometryShader(), sizeof(GlobalPushConst));

  pipeline->SetInterface(GetInterface());

  pipeline->Init(ctx, GetVertexShader(), GetFragmentShader(),
                 GetGeometryShader());

  return pipeline;
}

std::unique_ptr<AbsPipelineWrapper>
ColorPipelineFamily::CreateStencilKeepPipeline(GPUVkContext* ctx) {
  auto pipeline = std::make_unique<StencilKeepColorPipeline>(
      UseGeometryShader(), sizeof(GlobalPushConst));

  pipeline->SetInterface(GetInterface());

  pipeline->Init(ctx, GetVertexShader(), GetFragmentShader(),
                 GetGeometryShader());

  return pipeline;
}

std::unique_ptr<AbsPipelineWrapper> ColorPipelineFamily::CreateOSStaticPipeline(
    GPUVkContext* ctx) {
  auto pipeline = std::make_unique<StaticColorPipeline>(
      UseGeometryShader(), sizeof(GlobalPushConst));

  pipeline->SetInterface(GetInterface());

  pipeline->SetRenderPass(OffScreenRenderPass());

  pipeline->Init(ctx, GetVertexShader(), GetFragmentShader(),
                 GetGeometryShader());

  return pipeline;
}

std::unique_ptr<AbsPipelineWrapper>
ColorPipelineFamily::CreateOSStencilPipeline(GPUVkContext* ctx) {
  auto pipeline = std::make_unique<StencilDiscardColorPipeline>(
      UseGeometryShader(), sizeof(GlobalPushConst));

  pipeline->SetInterface(GetInterface());

  pipeline->SetRenderPass(OffScreenRenderPass());

  pipeline->Init(ctx, GetVertexShader(), GetFragmentShader(),
                 GetGeometryShader());

  return pipeline;
}

std::unique_ptr<PipelineFamily> PipelineFamily::CreateColorPipelineFamily() {
  return std::make_unique<ColorPipelineFamily>();
}

}  // namespace skity