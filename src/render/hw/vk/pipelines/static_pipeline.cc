#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

VkPipelineDepthStencilStateCreateInfo
StaticPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  // static pipeline disable stencil test and not modify stencil buffer
  depth_stencil_state.stencilTestEnable = VK_FALSE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_ALWAYS;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

}  // namespace skity