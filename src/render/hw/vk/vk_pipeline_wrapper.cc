#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

#include <array>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
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
      VKUtils::PipelineMultisampleStateCreateInfo(ctx->GetSampleCount());
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

  if (VK_CALL(vkCreateGraphicsPipelines, ctx->GetDevice(), VK_NULL_HANDLE, 1,
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

void VKPipelineWrapper::UploadPushConstant(GlobalPushConst const& push_const,
                                           VkCommandBuffer cmd) {
  VK_CALL(vkCmdPushConstants, cmd, pipeline_layout_,
          VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
          sizeof(GlobalPushConst), &push_const);
}

void VKPipelineWrapper::UploadCommonSet(CommonFragmentSet const& common_set,
                                        GPUVkContext* ctx,
                                        VKFrameBuffer* frame_buffer,
                                        VKMemoryAllocator* allocator) {
  auto buffer = frame_buffer->ObtainCommonSetBuffer();
  allocator->UploadBuffer(buffer, (void*)&common_set,
                          sizeof(CommonFragmentSet));

  // common set is in set 1
  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, descriptor_set_layout_[1]);

  VkDescriptorBufferInfo buffer_info{buffer->GetBuffer(), 0,
                                     sizeof(CommonFragmentSet)};

  auto write_set = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &buffer_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), 1, &write_set, 0,
          VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, ctx->GetCurrentCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 1, 1,
          &descriptor_set, 0, nullptr);
}

void VKPipelineWrapper::UploadFontSet(VkDescriptorSet set, GPUVkContext* ctx) {
  if (GetFontSetLayout() == VK_NULL_HANDLE) {
    return;
  }

  VK_CALL(vkCmdBindDescriptorSets, ctx->GetCurrentCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 3, 1, &set, 0,
          nullptr);
}

void VKPipelineWrapper::UploadTransformMatrix(glm::mat4 const& matrix,
                                              GPUVkContext* ctx,
                                              VKFrameBuffer* frame_buffer,
                                              VKMemoryAllocator* allocator) {
  auto buffer = frame_buffer->ObtainTransformBuffer();
  allocator->UploadBuffer(buffer, (void*)&matrix, sizeof(glm::mat4));

  // transform matrix is in set 0
  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, descriptor_set_layout_[0]);

  VkDescriptorBufferInfo buffer_info{buffer->GetBuffer(), 0, sizeof(glm::mat4)};

  // create VkWriteDescriptorSet to update set
  auto write_set = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &buffer_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), 1, &write_set, 0,
          VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, ctx->GetCurrentCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
          &descriptor_set, 0, nullptr);
}

void VKPipelineWrapper::InitDescriptorSetLayout(GPUVkContext* ctx) {
  // create set 0
  auto set0_binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto set0_create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(&set0_binding, 1);

  descriptor_set_layout_[0] =
      VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), set0_create_info);

  // create set 1
  auto set1_binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto set1_create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(&set1_binding, 1);

  descriptor_set_layout_[1] =
      VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), set1_create_info);

  // create set 2
  // set 2 is create by sub class implementation
  descriptor_set_layout_[2] = GenerateColorSetLayout(ctx);

  // if subclass dose not create color set, then no need to create font set
  if (descriptor_set_layout_[2] == VK_NULL_HANDLE) {
    return;
  }

  auto set3_binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
      0);

  auto set3_create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(&set3_binding, 1);

  descriptor_set_layout_[3] =
      VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), set3_create_info);
}

void VKPipelineWrapper::InitPipelineLayout(GPUVkContext* ctx) {
  VkPushConstantRange push_const_range = VKUtils::PushConstantRange(
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      push_const_size_, 0);

  VkPipelineLayoutCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  uint32_t descriptor_set_count = descriptor_set_layout_.size();
  // stencil pipeline not use set 2
  if (descriptor_set_layout_[2] == VK_NULL_HANDLE) {
    descriptor_set_count -= 2;
  }

  create_info.setLayoutCount = descriptor_set_count;
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

VkPipelineDepthStencilStateCreateInfo VKPipelineWrapper::StencilDiscardInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_NOT_EQUAL;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x00;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo
VKPipelineWrapper::StencilClipDiscardInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_LESS;
  depth_stencil_state.front.compareMask = 0x1F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x10;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo VKPipelineWrapper::StencilKeepInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_KEEP;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_EQUAL;
  depth_stencil_state.front.compareMask = 0x1F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x10;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

}  // namespace skity