#include "src/render/hw/vk/pipelines/stencil_pipeline.hpp"

#include "shader.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilFrontPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilFrontPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_stencil_discard_frag_spv,
      vk_stencil_discard_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilClipFrontPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilClipFrontPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_stencil_discard_frag_spv,
      vk_stencil_discard_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper> VKPipelineWrapper::CreateStencilBackPipeline(
    GPUVkContext* ctx) {
  return PipelineBuilder<StencilBackPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_stencil_discard_frag_spv,
      vk_stencil_discard_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilClipBackPipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilClipBackPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_stencil_discard_frag_spv,
      vk_stencil_discard_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper> VKPipelineWrapper::CreateStencilClipPipeline(
    GPUVkContext* ctx) {
  return PipelineBuilder<StencilClipPipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_stencil_discard_frag_spv,
      vk_stencil_discard_frag_spv_size,
      ctx,
  }();
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilReplacePipeline(GPUVkContext* ctx) {
  return PipelineBuilder<StencilReplacePipeline>{
      (const char*)vk_common_vert_spv,
      vk_common_vert_spv_size,
      (const char*)vk_stencil_discard_frag_spv,
      vk_stencil_discard_frag_spv_size,
      ctx,
  }();
}

VkDescriptorSetLayout StencilPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  return VK_NULL_HANDLE;
}

VkPipelineColorBlendAttachmentState StencilPipeline::GetColorBlendState() {
  // all stencil pipeline no need color write
  VkPipelineColorBlendAttachmentState blend_attachment_state{};
  blend_attachment_state.blendEnable = VK_TRUE;
  blend_attachment_state.colorWriteMask = 0;
  blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blend_attachment_state.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
  blend_attachment_state.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

  return blend_attachment_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilFrontPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_ALWAYS;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x01;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipFrontPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depth_stencil_state.front.compareMask = 0x10;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x1F;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilBackPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_ALWAYS;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x01;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipBackPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depth_stencil_state.front.compareMask = 0x10;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x1F;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_NOT_EQUAL;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0xFF;
  depth_stencil_state.front.reference = 0x10;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

void StencilReplacePipeline::UpdateStencilInfo(uint32_t reference,
                                               GPUVkContext* ctx) {
  VK_CALL(vkCmdSetStencilReference, ctx->GetCurrentCMD(),
          VK_STENCIL_FACE_FRONT_AND_BACK, reference);
}

std::vector<VkDynamicState> StencilReplacePipeline::GetDynamicStates() {
  auto dynamic_state = StencilPipeline::GetDynamicStates();

  dynamic_state.emplace_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

  return dynamic_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilReplacePipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_ALWAYS;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0xFF;
  depth_stencil_state.front.reference = 0x10;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

}  // namespace skity