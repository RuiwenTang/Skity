#include "src/render/hw/vk/pipelines/stencil_pipeline.hpp"

#include "shader.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

VkDescriptorSetLayout StencilPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  return VK_NULL_HANDLE;
}

VkPipelineColorBlendAttachmentState StencilPipeline::GetColorBlendState() {
  // all stencil pipeline no need color write
  VkPipelineColorBlendAttachmentState blend_attachment_state{};
  blend_attachment_state.blendEnable = VK_FALSE;
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
StencilRecursiveClipBackPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_EQUAL;
  depth_stencil_state.front.compareMask = 0xFF;
  depth_stencil_state.front.writeMask = 0xFF;
  depth_stencil_state.front.reference = 0x10;
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

VkPipelineDepthStencilStateCreateInfo
StencilRecursiveClipPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_NOT_EQUAL;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x00;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

void StencilReplacePipeline::UpdateStencilInfo(uint32_t reference,
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

std::vector<VkDynamicState> StencilReplacePipeline::GetDynamicStates() {
  auto dynamic_state = StencilPipeline::GetDynamicStates();

  dynamic_state.emplace_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
  dynamic_state.emplace_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
  dynamic_state.emplace_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);

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

AbsPipelineWrapper* StencilPipelineFamily::ChoosePipeline(bool enable_stencil,
                                                          bool off_screen) {
  if (!enable_stencil) {
    return nullptr;
  }

  if (off_screen) {
    return PickOS();
  }

  if (StencilOp() == HWStencilOp::INCR_WRAP) {
    return PickFront();
  }

  if (StencilOp() == HWStencilOp::DECR_WRAP) {
    return PickBack();
  }

  if (StencilOp() == HWStencilOp::REPLACE) {
    return PickReplace();
  }

  return nullptr;
}

void StencilPipelineFamily::OnInit(GPUVkContext* ctx) {
  // shader
  VkShaderModule vs_shader = VK_NULL_HANDLE;
  VkShaderModule fs_shader = VK_NULL_HANDLE;
  VkShaderModule gs_shader = VK_NULL_HANDLE;

  std::tie(vs_shader, fs_shader, gs_shader) = GenerateShader(ctx);

  front_ = CreatePipeline<StencilFrontPipeline>(ctx, vs_shader, fs_shader,
                                                gs_shader);
  os_front_ = CreatePipeline<StencilFrontPipeline>(
      ctx, vs_shader, fs_shader, gs_shader, OffScreenRenderPass());
  clip_front_ = CreatePipeline<StencilClipFrontPipeline>(ctx, vs_shader,
                                                         fs_shader, gs_shader);

  back_ =
      CreatePipeline<StencilBackPipeline>(ctx, vs_shader, fs_shader, gs_shader);
  os_back_ = CreatePipeline<StencilBackPipeline>(
      ctx, vs_shader, fs_shader, gs_shader, OffScreenRenderPass());
  clip_back_ = CreatePipeline<StencilClipBackPipeline>(ctx, vs_shader,
                                                       fs_shader, gs_shader);

  clip_ =
      CreatePipeline<StencilClipPipeline>(ctx, vs_shader, fs_shader, gs_shader);

  recursive_ = CreatePipeline<StencilRecursiveClipPipeline>(
      ctx, vs_shader, fs_shader, gs_shader);
  recursive_back_ = CreatePipeline<StencilRecursiveClipBackPipeline>(
      ctx, vs_shader, fs_shader, gs_shader);

  replace_ = CreatePipeline<StencilReplacePipeline>(ctx, vs_shader, fs_shader,
                                                    gs_shader);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vs_shader, VK_NULL_HANDLE);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fs_shader, VK_NULL_HANDLE);
  if (gs_shader) {
    VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), gs_shader, VK_NULL_HANDLE);
  }
}

void StencilPipelineFamily::OnDestroy(GPUVkContext* ctx) {
  SAFE_DESTROY(front_, ctx);
  SAFE_DESTROY(os_front_, ctx);
  SAFE_DESTROY(clip_front_, ctx);

  SAFE_DESTROY(back_, ctx);
  SAFE_DESTROY(os_back_, ctx);
  SAFE_DESTROY(clip_back_, ctx);

  SAFE_DESTROY(clip_, ctx);

  SAFE_DESTROY(recursive_, ctx);
  SAFE_DESTROY(recursive_back_, ctx);

  SAFE_DESTROY(replace_, ctx);
}

std::tuple<VkShaderModule, VkShaderModule, VkShaderModule>
StencilPipelineFamily::GenerateShader(GPUVkContext* ctx) {
  VkShaderModule vs_shader = VK_NULL_HANDLE;
  VkShaderModule fs_shader = VK_NULL_HANDLE;
  VkShaderModule gs_shader = VK_NULL_HANDLE;

  if (UseGeometryShader()) {
    vs_shader = VKUtils::CreateShader(GetInterface(), ctx->GetDevice(),
                                      (const char*)vk_gs_common_vert_spv,
                                      vk_gs_common_vert_spv_size);
  } else {
    vs_shader = VKUtils::CreateShader(GetInterface(), ctx->GetDevice(),
                                      (const char*)vk_common_vert_spv,
                                      vk_common_vert_spv_size);
  }

  fs_shader = VKUtils::CreateShader(GetInterface(), ctx->GetDevice(),
                                    (const char*)vk_stencil_discard_frag_spv,
                                    vk_stencil_discard_frag_spv_size);

  if (UseGeometryShader()) {
    gs_shader = VKUtils::CreateShader(GetInterface(), ctx->GetDevice(),
                                      (const char*)vk_gs_geometry_geom_spv,
                                      vk_gs_geometry_geom_spv_size);
  }

  return {vs_shader, fs_shader, gs_shader};
}

AbsPipelineWrapper* StencilPipelineFamily::PickOS() {
  if (StencilOp() == HWStencilOp::INCR_WRAP) {
    return os_front_.get();
  } else if (StencilOp() == HWStencilOp::DECR_WRAP) {
    return os_back_.get();
  }

  return nullptr;
}

AbsPipelineWrapper* StencilPipelineFamily::PickFront() {
  if (StencilFunc() == HWStencilFunc::ALWAYS) {
    return front_.get();
  }

  return clip_front_.get();
}

AbsPipelineWrapper* StencilPipelineFamily::PickBack() {
  if (StencilFunc() == HWStencilFunc::ALWAYS) {
    return back_.get();
  } else if (StencilFunc() == HWStencilFunc::LESS_OR_EQUAL) {
    return clip_back_.get();
  } else if (StencilFunc() == HWStencilFunc::EQUAL) {
    return recursive_back_.get();
  }

  return nullptr;
}

AbsPipelineWrapper* StencilPipelineFamily::PickReplace() {
  if (StencilFunc() == HWStencilFunc::ALWAYS) {
    return replace_.get();
  }

  if (StencilFunc() == HWStencilFunc::NOT_EQUAL) {
    if (WriteMask() == 0xFF) {
      return clip_.get();
    } else {
      return recursive_.get();
    }
  }

  return nullptr;
}

std::unique_ptr<PipelineFamily> PipelineFamily::CreateStencilPipelineFamily() {
  return std::make_unique<StencilPipelineFamily>();
}

}  // namespace skity