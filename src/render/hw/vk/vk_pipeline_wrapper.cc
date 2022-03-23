#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

#include <array>
#include <vector>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_texture.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

void RenderPipeline::Init(GPUVkContext* ctx, VkShaderModule vertex,
                          VkShaderModule fragment, VkShaderModule geometry) {
  std::vector<VkPipelineShaderStageCreateInfo> shaders{};

  VkPipelineShaderStageCreateInfo vs_ci{};
  vs_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vs_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vs_ci.module = vertex;
  vs_ci.pName = "main";

  shaders.emplace_back(vs_ci);

  VkPipelineShaderStageCreateInfo fs_ci{};
  fs_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fs_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fs_ci.module = fragment;
  fs_ci.pName = "main";

  shaders.emplace_back(fs_ci);

  if (UseGeometryShader()) {
    VkPipelineShaderStageCreateInfo gs_ci{};
    gs_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    gs_ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    gs_ci.module = geometry;
    gs_ci.pName = "main";

    shaders.emplace_back(gs_ci);
  }

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

  auto pipeline_create_info = VKUtils::PipelineCreateInfo(
      pipeline_layout_,
      os_render_pass_ ? os_render_pass_ : ctx->GetRenderPass());

  if (os_render_pass_) {
    // currently off screen do not have multisample
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  }

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

void RenderPipeline::Destroy(GPUVkContext* ctx) {
  VK_CALL(vkDestroyPipeline, ctx->GetDevice(), pipeline_, nullptr);
  VK_CALL(vkDestroyPipelineLayout, ctx->GetDevice(), pipeline_layout_, nullptr);

  for (auto set_layout : descriptor_set_layout_) {
    VK_CALL(vkDestroyDescriptorSetLayout, ctx->GetDevice(), set_layout,
            nullptr);
  }
}

void RenderPipeline::Bind(VkCommandBuffer cmd) {
  VK_CALL(vkCmdBindPipeline, cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
  bind_cmd_ = cmd;
}

void RenderPipeline::UploadPushConstant(GlobalPushConst const& push_const,
                                        VkCommandBuffer cmd) {
  VK_CALL(vkCmdPushConstants, cmd, pipeline_layout_,
          VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
          sizeof(GlobalPushConst), &push_const);
}

void RenderPipeline::UploadCommonSet(CommonFragmentSet const& common_set,
                                     GPUVkContext* ctx,
                                     SKVkFrameBufferData* frame_buffer,
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

  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 1, 1,
          &descriptor_set, 0, nullptr);
}

void RenderPipeline::UploadFontSet(VkDescriptorSet set, GPUVkContext* ctx) {
  if (GetFontSetLayout() == VK_NULL_HANDLE) {
    return;
  }

  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 3, 1, &set, 0,
          nullptr);
}

void RenderPipeline::UploadTransformMatrix(glm::mat4 const& matrix,
                                           GPUVkContext* ctx,
                                           SKVkFrameBufferData* frame_buffer,
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

  VK_CALL(vkCmdBindDescriptorSets, GetBindCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
          &descriptor_set, 0, nullptr);
}

void RenderPipeline::InitDescriptorSetLayout(GPUVkContext* ctx) {
  VkShaderStageFlags set0_stage_flags;
  if (UseGeometryShader()) {
    set0_stage_flags =
        VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  } else {
    set0_stage_flags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  // create set 0
  auto set0_binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, set0_stage_flags, 0);

  auto set0_create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(&set0_binding, 1);

  descriptor_set_layout_[0] = VKUtils::CreateDescriptorSetLayout(
      GetInterface(), ctx->GetDevice(), set0_create_info);

  VkShaderStageFlags set1_stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;
  if (UseGeometryShader()) {
    set1_stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
  }
  // create set 1
  auto set1_binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, set1_stage_flags, 0);

  auto set1_create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(&set1_binding, 1);

  descriptor_set_layout_[1] = VKUtils::CreateDescriptorSetLayout(
      GetInterface(), ctx->GetDevice(), set1_create_info);

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

  descriptor_set_layout_[3] = VKUtils::CreateDescriptorSetLayout(
      GetInterface(), ctx->GetDevice(), set3_create_info);
}

void RenderPipeline::InitPipelineLayout(GPUVkContext* ctx) {
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

VkPipelineColorBlendAttachmentState RenderPipeline::GetColorBlendState() {
  VkPipelineColorBlendAttachmentState blend_attachment_state{};

  blend_attachment_state.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  if (os_render_pass_) {
    blend_attachment_state.blendEnable = VK_FALSE;
    blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment_state.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  } else {
    blend_attachment_state.blendEnable = VK_TRUE;
    blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend_attachment_state.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }

  return blend_attachment_state;
}

std::vector<VkDynamicState> RenderPipeline::GetDynamicStates() {
  std::vector<VkDynamicState> states{
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };

  return states;
}

VkVertexInputBindingDescription RenderPipeline::GetVertexInputBinding() {
  VkVertexInputBindingDescription input_binding{};
  input_binding.binding = 0;
  input_binding.stride = 5 * sizeof(float);

  return input_binding;
}

std::array<VkVertexInputAttributeDescription, 2>
RenderPipeline::GetVertexInputAttributes() {
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

VkPipelineDepthStencilStateCreateInfo RenderPipeline::StencilDiscardInfo() {
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

VkPipelineDepthStencilStateCreateInfo RenderPipeline::StencilLessDiscardInfo() {
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

VkPipelineDepthStencilStateCreateInfo RenderPipeline::StencilKeepInfo() {
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

void ComputePipeline::Init(GPUVkContext* ctx, VkShaderModule vertex,
                           VkShaderModule fragment, VkShaderModule geometry) {
  InitPipelineLayout(ctx);

  VkPipelineShaderStageCreateInfo shader_stage{
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  shader_stage.module = vertex;
  shader_stage.pName = "main";

  auto pipeline_ci = VKUtils::ComputePipelineCreateInfo(pipeline_layout_);
  pipeline_ci.stage = shader_stage;

  VK_CALL(vkCreateComputePipelines, ctx->GetDevice(), VK_NULL_HANDLE, 1,
          &pipeline_ci, nullptr, &pipeline_);
}

void ComputePipeline::Destroy(GPUVkContext* ctx) {
  VK_CALL(vkDestroyPipeline, ctx->GetDevice(), pipeline_, nullptr);
  VK_CALL(vkDestroyPipelineLayout, ctx->GetDevice(), pipeline_layout_, nullptr);
  VK_CALL(vkDestroyDescriptorSetLayout, ctx->GetDevice(),
          descriptor_set_layout_, nullptr);
}

void ComputePipeline::Bind(VkCommandBuffer cmd) {
  VK_CALL(vkCmdBindPipeline, cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
  bind_cmd_ = cmd;
}

void ComputePipeline::Dispatch(VkCommandBuffer cmd, GPUVkContext* ctx) {
  auto out_texture = OutpuTexture();
  if (out_texture == nullptr) {
    return;
  }

  OnDispatch(cmd, ctx);
  VK_CALL(vkCmdDispatch, cmd,
          (out_texture->GetWidth() + LOCAL_SIZE - 1) / LOCAL_SIZE,
          (out_texture->GetHeight() + LOCAL_SIZE - 1) / LOCAL_SIZE, 1);
}

void ComputePipeline::UploadOutputTexture(VKTexture* texture) {
  output_texture_ = texture;
}

void ComputePipeline::UploadCommonSet(const CommonFragmentSet& common_set,
                                      GPUVkContext* ctx,
                                      SKVkFrameBufferData* frame_buffer,
                                      VKMemoryAllocator* allocator) {
  common_info_ = common_set.info;
  UpdateFrameDataAndAllocator(frame_buffer, allocator);
}

void ComputePipeline::UploadGradientInfo(const GradientInfo& info,
                                         GPUVkContext* ctx,
                                         SKVkFrameBufferData* frame_buffer,
                                         VKMemoryAllocator* allocator) {
  bounds_info_ = info.bounds;
  UpdateFrameDataAndAllocator(frame_buffer, allocator);
}

void ComputePipeline::UploadImageTexture(VKTexture* texture, GPUVkContext* ctx,
                                         SKVkFrameBufferData* frame_buffer,
                                         VKMemoryAllocator* allocator) {
  input_texture_ = texture;
  UpdateFrameDataAndAllocator(frame_buffer, allocator);
}

void ComputePipeline::InitPipelineLayout(GPUVkContext* ctx) {
  descriptor_set_layout_ = CreateDescriptorSetLayout(ctx);

  VkPipelineLayoutCreateInfo pipeline_ci{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  pipeline_ci.setLayoutCount = 1;
  pipeline_ci.pSetLayouts = &descriptor_set_layout_;

  VK_CALL(vkCreatePipelineLayout, ctx->GetDevice(), &pipeline_ci, nullptr,
          &pipeline_layout_);
}

void RenderPipelineFamily::OnInit(GPUVkContext* ctx) {
  vs_shader_ = GenerateVertexShader(ctx);
  fs_shader_ = GenerateFragmentShader(ctx);
  gs_shader_ = GenerateGeometryShader(ctx);

  static_pipeline_ = CreateStaticPipeline(ctx);
  stencil_discard_pipeline_ = CreateStencilDiscardPipeline(ctx);
  stencil_clip_pipeline_ = CreateStencilClipPipeline(ctx);
  stencil_keep_pipeline_ = CreateStencilKeepPipeline(ctx);

  if (OffScreenRenderPass()) {
    os_static_pipeline_ = CreateOSStaticPipeline(ctx);
    os_stencil_pipeline_ = CreateOSStencilPipeline(ctx);
  }

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vs_shader_, VK_NULL_HANDLE);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fs_shader_, VK_NULL_HANDLE);
  if (UseGeometryShader()) {
    VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), gs_shader_,
            VK_NULL_HANDLE);
  }
}

void RenderPipelineFamily::OnDestroy(GPUVkContext* ctx) {
  SAFE_DESTROY(static_pipeline_, ctx);
  SAFE_DESTROY(stencil_discard_pipeline_, ctx);
  SAFE_DESTROY(stencil_clip_pipeline_, ctx);
  SAFE_DESTROY(stencil_keep_pipeline_, ctx);
  SAFE_DESTROY(os_static_pipeline_, ctx);
  SAFE_DESTROY(os_stencil_pipeline_, ctx);
}

AbsPipelineWrapper* RenderPipelineFamily::ChoosePipeline(bool enable_stencil,
                                                         bool off_screen) {
  if (off_screen) {
    return ChooseOffScreenPiepline(enable_stencil);
  } else {
    return ChooseRenderPipeline(enable_stencil);
  }
}

VkShaderModule RenderPipelineFamily::GenerateGeometryShader(GPUVkContext* ctx) {
  if (!UseGeometryShader()) {
    return VK_NULL_HANDLE;
  }

  return VKUtils::CreateShader(GetInterface(), ctx->GetDevice(),
                               (const char*)vk_gs_geometry_geom_spv,
                               vk_gs_geometry_geom_spv_size);
}

AbsPipelineWrapper* RenderPipelineFamily::ChooseOffScreenPiepline(
    bool enable_stencil) {
  if (enable_stencil) {
    return os_stencil_pipeline_.get();
  } else {
    return os_static_pipeline_.get();
  }
}

AbsPipelineWrapper* RenderPipelineFamily::ChooseRenderPipeline(
    bool enable_stencil) {
  if (!enable_stencil) {
    return static_pipeline_.get();
  }

  if (StencilFunc() == HWStencilFunc::NOT_EQUAL) {
    return stencil_discard_pipeline_.get();
  } else if (StencilFunc() == HWStencilFunc::LESS) {
    return stencil_clip_pipeline_.get();
  } else if (StencilFunc() == HWStencilFunc::EQUAL) {
    return stencil_keep_pipeline_.get();
  }

  return nullptr;
}

std::tuple<const char*, size_t> RenderPipelineFamily::GetVertexShaderInfo() {
  if (UseGeometryShader()) {
    return {(const char*)vk_gs_common_vert_spv, vk_gs_common_vert_spv_size};
  } else {
    return {(const char*)vk_common_vert_spv, vk_common_vert_spv_size};
  }
}

VkShaderModule RenderPipelineFamily::GenerateVertexShader(GPUVkContext* ctx) {
  const char* shader_source;
  size_t shader_size;

  std::tie(shader_source, shader_size) = GetVertexShaderInfo();

  return VKUtils::CreateShader(GetInterface(), ctx->GetDevice(), shader_source,
                               shader_size);
}

VkShaderModule RenderPipelineFamily::GenerateFragmentShader(GPUVkContext* ctx) {
  const char* shader_source;
  size_t shader_size;

  std::tie(shader_source, shader_size) = GetFragmentShaderInfo();

  return VKUtils::CreateShader(GetInterface(), ctx->GetDevice(), shader_source,
                               shader_size);
}

}  // namespace skity