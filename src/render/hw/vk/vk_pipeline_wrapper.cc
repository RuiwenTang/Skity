#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

#include <array>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

void VKPipelineWrapper::Init(GPUVkContext* ctx, VkShaderModule vertex,
                             VkShaderModule fragment) {
  std::array<VkPipelineShaderStageCreateInfo, 2> shaders{};
  shaders[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaders[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaders[0].module = vertex;
  shaders[0].pName = "main";

  shaders[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaders[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaders[1].module = fragment;
  shaders[1].pName = "main";

  InitDescriptorSetLayout(ctx);
  InitPipelineLayout(ctx);

  // all pipeline use single input binding with 2 attributes
  auto input_binding = GetVertexInputBinding();
  // input attributes
  auto input_attr = GetVertexInputAttributes();
  auto vertex_input_state = VKUtils::PipelineVertexInputStateCreateInfo();
  vertex_input_state.vertexBindingDescriptionCount = 1;
  vertex_input_state.pVertexBindingDescriptions = &input_binding;
  vertex_input_state.vertexAttributeDescriptionCount = input_attr.size();
  vertex_input_state.pVertexAttributeDescriptions = input_attr.data();

  auto input_assembly_state = VKUtils::PipelineInputAssemblyStateCreateInfo(
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

  auto rasterization_state = VKUtils::PipelineRasterizationStateCreateInfo(
      VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

  auto color_blend_attachment = GetColorBlendState();
  auto color_blend_state =
      VKUtils::PipelineColorBlendStateCreateInfo(1, &color_blend_attachment);
  auto depth_stencil_state = GetDepthStencilStateCreateInfo();
  auto view_port_state = VKUtils::PipelineViewportStateCreateInfo(1, 1);
  // TODO support multisample for vulkan
  auto multisample_state =
      VKUtils::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
  auto dynamic_states_value = GetDynamicStates();
  auto dynamic_state =
      VKUtils::PipelineDynamicStateCreateInfo(dynamic_states_value);

  auto pipeline_create_info =
      VKUtils::PipelineCreateInfo(pipeline_layout_, ctx->GetRenderPass());

  pipeline_create_info.pVertexInputState = &vertex_input_state;
  pipeline_create_info.pInputAssemblyState = &input_assembly_state;
  pipeline_create_info.pRasterizationState = &rasterization_state;
  pipeline_create_info.pColorBlendState = &color_blend_state;
  pipeline_create_info.pMultisampleState = &multisample_state;
  pipeline_create_info.pViewportState = &view_port_state;
  pipeline_create_info.pDepthStencilState = &depth_stencil_state;
  pipeline_create_info.pDynamicState = &dynamic_state;
  pipeline_create_info.stageCount = shaders.size();
  pipeline_create_info.pStages = shaders.data();

  if (VK_CALL(vkCreateGraphicsPipelines, ctx->GetDevice(), nullptr, 1,
              &pipeline_create_info, nullptr, &pipeline_) != VK_SUCCESS) {
    LOG_ERROR("Failed to create Graphic Pipeline");
  }
}

void VKPipelineWrapper::Destroy(GPUVkContext* ctx) {
  VK_CALL(vkDestroyPipeline, ctx->GetDevice(), pipeline_, nullptr);
  VK_CALL(vkDestroyPipelineLayout, ctx->GetDevice(), pipeline_layout_, nullptr);

  for (auto set_layout : descriptor_set_layout_) {
    VK_CALL(vkDestroyDescriptorSetLayout, ctx->GetDevice(), set_layout,
            nullptr);
  }
}

void VKPipelineWrapper::Bind(VkCommandBuffer cmd) {
  VK_CALL(vkCmdBindPipeline, cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}

void VKPipelineWrapper::InitDescriptorSetLayout(GPUVkContext* ctx) {
  descriptor_set_layout_ = GenearteDescriptorSetLayout(ctx);
}

std::vector<VkDescriptorSetLayout>
VKPipelineWrapper::GenearteDescriptorSetLayout(GPUVkContext* ctx) {
  std::vector<VkDescriptorSetLayout> set_layout{};

  // set 0 is common for all pipelines
  auto binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto set_create_info = VKUtils::DescriptorSetLayoutCreateInfo(&binding, 1);

  set_layout.emplace_back(
      VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), set_create_info));

  return set_layout;
}

void VKPipelineWrapper::InitPipelineLayout(GPUVkContext* ctx) {
  VkPushConstantRange push_const_range = VKUtils::PushConstantRange(
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      push_const_size_, 0);

  VkPipelineLayoutCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  create_info.setLayoutCount = descriptor_set_layout_.size();
  create_info.pSetLayouts = descriptor_set_layout_.data();
  create_info.pushConstantRangeCount = 1;
  create_info.pPushConstantRanges = &push_const_range;

  if (VK_CALL(vkCreatePipelineLayout, ctx->GetDevice(), &create_info, nullptr,
              &pipeline_layout_) != VK_SUCCESS) {
    LOG_ERROR("Failed to create pipeline layout for {} descriptor sets",
              descriptor_set_layout_.size());
  }
}

VkPipelineColorBlendAttachmentState VKPipelineWrapper::GetColorBlendState() {
  VkPipelineColorBlendAttachmentState blend_attachment_state{};
  blend_attachment_state.blendEnable = VK_TRUE;
  blend_attachment_state.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

std::vector<VkDynamicState> VKPipelineWrapper::GetDynamicStates() {
  std::vector<VkDynamicState> states{
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };

  return states;
}

VkVertexInputBindingDescription VKPipelineWrapper::GetVertexInputBinding() {
  VkVertexInputBindingDescription input_binding{};
  input_binding.binding = 0;
  input_binding.stride = 5 * sizeof(float);

  return input_binding;
}

std::array<VkVertexInputAttributeDescription, 2>
VKPipelineWrapper::GetVertexInputAttributes() {
  std::array<VkVertexInputAttributeDescription, 2> input_attr{};

  // location 0 vec2 [x, y]
  input_attr[0].binding = 0;
  input_attr[0].location = 0;
  input_attr[0].format = VK_FORMAT_R32G32_SFLOAT;
  input_attr[0].offset = 0;
  // location 1 vec3 [mix, u, v]
  input_attr[1].binding = 0;
  input_attr[1].location = 1;
  input_attr[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  input_attr[1].offset = 2 * sizeof(float);

  return input_attr;
}

}  // namespace skity