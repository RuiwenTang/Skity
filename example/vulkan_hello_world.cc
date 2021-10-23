#include <cstring>
#include <example_config.hpp>
#include <iostream>
#include <skity/codec/data.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "utils/vk_app.hpp"

class HelloVulkanApp : public example::VkApp {
 public:
  HelloVulkanApp() : VkApp(800, 800, "Hello Vulkan") {}
  ~HelloVulkanApp() override = default;

 protected:
  void OnCreate() override { SetupVkContext(); }
  void OnUpdate(float elapsed_time) override {
    GetVkCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics,
                                      vk_pipeline_, GetVkDispatch());

    GetVkCommandBuffer().bindVertexBuffers(0, vk_vertex_buffer_, {0});

    GetVkCommandBuffer().draw(3, 1, 0, 0);
  }
  void OnDestroy() override { DestroyVkContext(); }

 private:
  void SetupVkContext() {
    CreatePipeline();
    CreateVertexBuffer();
  }
  void DestroyVkContext() {
    GetVkDevice().destroyPipeline(vk_pipeline_, nullptr, GetVkDispatch());
    GetVkDevice().destroyPipelineLayout(vk_pipeline_layout_, nullptr,
                                        GetVkDispatch());

    GetVkDevice().destroyBuffer(vk_vertex_buffer_, nullptr, GetVkDispatch());
    GetVkDevice().freeMemory(vk_vertex_buffer_memory_, nullptr,
                             GetVkDispatch());
  }

  void CreatePipeline() {
    auto vertex_shader_data = skity::Data::MakeFromFileName(
        EXAMPLE_VK_SHADER_ROOT "vk_demo_buffer_shader_vert.spv");
    auto frag_shader_data = skity::Data::MakeFromFileName(
        EXAMPLE_VK_SHADER_ROOT "vk_demo_shader_frag.spv");

    assert(!vertex_shader_data->IsEmpty());
    assert(!frag_shader_data->IsEmpty());

    auto vert_shader_module_ret = GetVkDevice().createShaderModuleUnique(
        {{},
         vertex_shader_data->Size(),
         (const uint32_t*)vertex_shader_data->RawData()},
        nullptr, GetVkDispatch());
    assert(vert_shader_module_ret.result == vk::Result::eSuccess);

    auto frag_shader_module_ret = GetVkDevice().createShaderModuleUnique(
        {{},
         frag_shader_data->Size(),
         (const uint32_t*)frag_shader_data->RawData()},
        nullptr, GetVkDispatch());
    assert(frag_shader_module_ret.result == vk::Result::eSuccess);

    std::vector<vk::PipelineShaderStageCreateInfo>
        pipeline_shader_stage_create_info;

    pipeline_shader_stage_create_info.emplace_back(
        vk::PipelineShaderStageCreateInfo{{},
                                          vk::ShaderStageFlagBits::eVertex,
                                          vert_shader_module_ret.value.get(),
                                          "main"});
    pipeline_shader_stage_create_info.emplace_back(
        vk::PipelineShaderStageCreateInfo{{},
                                          vk::ShaderStageFlagBits::eFragment,
                                          frag_shader_module_ret.value.get(),
                                          "main"});

    // description for vertex buffer
    vk::VertexInputBindingDescription vertex_binding_desc{
        0, 6 * sizeof(float), vk::VertexInputRate::eVertex};

    std::array<vk::VertexInputAttributeDescription, 2> vertex_input_attr_desc{};
    vertex_input_attr_desc[0].binding = 0;
    vertex_input_attr_desc[0].location = 0;
    vertex_input_attr_desc[0].format = vk::Format::eR32G32Sfloat;
    vertex_input_attr_desc[0].offset = 0;

    vertex_input_attr_desc[1].binding = 0;
    vertex_input_attr_desc[1].location = 1;
    vertex_input_attr_desc[1].format = vk::Format::eR32G32B32A32Sfloat;
    vertex_input_attr_desc[1].offset = 2 * sizeof(float);

    vk::PipelineInputAssemblyStateCreateInfo
        pipeline_input_assembly_state_create_info{
            {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};

    vk::PipelineVertexInputStateCreateInfo
        pipeline_vertex_input_state_create_info{
            {}, vertex_binding_desc, vertex_input_attr_desc};

    vk::Viewport view_port{
        0.f, 0.f, (float)GetFrameExtend().width, (float)GetFrameExtend().height,
        0.f, 1.f};

    vk::Rect2D scissor{vk::Offset2D(0, 0), GetFrameExtend()};

    vk::PipelineViewportStateCreateInfo pipeline_viewport_state_create_info{
        {}, 1, &view_port, 1, &scissor};

    vk::PipelineRasterizationStateCreateInfo
        pipeline_rasterization_state_create_info{{},
                                                 VK_FALSE,
                                                 VK_FALSE,
                                                 vk::PolygonMode::eFill,
                                                 vk::CullModeFlagBits::eNone,
                                                 vk::FrontFace::eClockwise,
                                                 VK_FALSE,
                                                 0.f,
                                                 0.f,
                                                 0.f,
                                                 1.f};

    vk::PipelineMultisampleStateCreateInfo
        pipeline_multisample_state_create_info{{}, vk::SampleCountFlagBits::e1};

    vk::StencilOpState stencil_op_state{
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep,
        vk::CompareOp::eAlways};

    vk::PipelineDepthStencilStateCreateInfo
        pipeline_depth_stencil_state_create_info{
            {},       VK_FALSE, VK_FALSE,         vk::CompareOp::eLess,
            VK_FALSE, VK_FALSE, stencil_op_state, stencil_op_state,
            0.f,      1.f};

    vk::ColorComponentFlags color_component_flags =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendAttachmentState pipeline_color_blend_attachment_state{
        VK_TRUE,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        color_component_flags};

    vk::PipelineColorBlendStateCreateInfo
        pipeline_color_blend_state_create_info{
            {},
            VK_FALSE,
            vk::LogicOp::eCopy,
            pipeline_color_blend_attachment_state,
            {1.f, 1.f, 1.f, 1.f}};

    vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};

    auto pipeline_layout_ret = GetVkDevice().createPipelineLayout(
        pipeline_layout_create_info, nullptr, GetVkDispatch());

    assert(pipeline_layout_ret.result == vk::Result::eSuccess);

    vk_pipeline_layout_ = pipeline_layout_ret.value;

    vk::GraphicsPipelineCreateInfo pipeline_create_info{
        {},
        pipeline_shader_stage_create_info,
        &pipeline_vertex_input_state_create_info,
        &pipeline_input_assembly_state_create_info,
        nullptr,
        &pipeline_viewport_state_create_info,
        &pipeline_rasterization_state_create_info,
        &pipeline_multisample_state_create_info,
        &pipeline_depth_stencil_state_create_info,
        &pipeline_color_blend_state_create_info,
        &pipeline_dynamic_state_create_info,
        vk_pipeline_layout_,
        GetAppRenderPass()};

    auto result = GetVkDevice().createGraphicsPipeline(
        nullptr, pipeline_create_info, nullptr, GetVkDispatch());

    assert(result.result == vk::Result::eSuccess);

    vk_pipeline_ = result.value;
  }

  void CreateVertexBuffer() {
    std::vector<float> vertex{
        0.f,   -0.5f, 1.f, 0.f, 1.f, 1.0f,  // vertex 1
        0.5f,  0.5f,  0.f, 1.f, 0.f, 0.5f,  // vertex 2
        -0.5f, 0.5f,  0.f, 0.f, 1.f, 0.0f   // vertex 3
    };

    vk::BufferCreateInfo buffer_create_info{
        {},
        vertex.size() * sizeof(float),
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive};

    auto buffer_create_ret = GetVkDevice().createBuffer(
        buffer_create_info, nullptr, GetVkDispatch());

    assert(buffer_create_ret.result == vk::Result::eSuccess);

    vk_vertex_buffer_ = buffer_create_ret.value;

    auto memory_requirements = GetVkDevice().getBufferMemoryRequirements(
        vk_vertex_buffer_, GetVkDispatch());

    vk::MemoryAllocateInfo memory_allocate_info{
        memory_requirements.size,
        FindMemoryType(memory_requirements.memoryTypeBits,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                           vk::MemoryPropertyFlagBits::eHostCoherent)};

    auto allocate_ret = GetVkDevice().allocateMemory(memory_allocate_info,
                                                     nullptr, GetVkDispatch());

    assert(allocate_ret.result == vk::Result::eSuccess);

    vk_vertex_buffer_memory_ = allocate_ret.value;

    void* data;
    GetVkDevice().mapMemory(vk_vertex_buffer_memory_, 0,
                            buffer_create_info.size, {}, &data,
                            GetVkDispatch());

    std::memcpy(data, vertex.data(), (size_t)buffer_create_info.size);

    GetVkDevice().unmapMemory(vk_vertex_buffer_memory_, GetVkDispatch());

    GetVkDevice().bindBufferMemory(vk_vertex_buffer_, vk_vertex_buffer_memory_,
                                   0, GetVkDispatch());
  }

 private:
  vk::PipelineLayout vk_pipeline_layout_;
  vk::Pipeline vk_pipeline_;
  vk::Buffer vk_vertex_buffer_;
  vk::DeviceMemory vk_vertex_buffer_memory_;
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
