#include <cstring>
#include <example_config.hpp>
#include <glm/glm.hpp>
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

    GetVkCommandBuffer().pushConstants(
        vk_pipeline_layout_, vk::ShaderStageFlagBits::eFragment, 0,
        sizeof(glm::vec4), &screen_info_, GetVkDispatch());

    GetVkCommandBuffer().bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, vk_pipeline_layout_, 0,
        vk_descriptor_set_, {}, GetVkDispatch());

    GetVkCommandBuffer().draw(6, 1, 0, 0);
  }
  void OnDestroy() override { DestroyVkContext(); }

 private:
  void SetupVkContext() {
    CreateDescriptorSetLayout();
    CreatePipeline();
    CreateVertexBuffer();
    CreateStencilImage();
    CreateDescriptorSet();
  }
  void DestroyVkContext() {
    GetVkDevice().destroyPipeline(vk_pipeline_, nullptr, GetVkDispatch());
    GetVkDevice().destroyPipelineLayout(vk_pipeline_layout_, nullptr,
                                        GetVkDispatch());

    GetVkDevice().destroyBuffer(vk_vertex_buffer_, nullptr, GetVkDispatch());
    GetVkDevice().freeMemory(vk_vertex_buffer_memory_, nullptr,
                             GetVkDispatch());

    GetVkDevice().destroyImage(vk_image_, nullptr, GetVkDispatch());
    GetVkDevice().freeMemory(vk_image_memory_, nullptr, GetVkDispatch());
    GetVkDevice().destroyImageView(vk_image_view_, nullptr, GetVkDispatch());
    for (auto descriptor_set_layout : vk_pipeline_descriptor_set_) {
      GetVkDevice().destroyDescriptorSetLayout(descriptor_set_layout, nullptr,
                                               GetVkDispatch());
    }

    GetVkDevice().destroyDescriptorPool(vk_descriptor_pool_, nullptr,
                                        GetVkDispatch());
  }

  void CreateDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> set_layout_bindings{};
    set_layout_bindings.emplace_back(
        vk::DescriptorSetLayoutBinding{0,
                                       vk::DescriptorType::eStorageImage,
                                       vk::ShaderStageFlagBits::eFragment,
                                       {}});
    set_layout_bindings[0].descriptorCount = 1;

    auto descriptor_set = GetVkDevice().createDescriptorSetLayout(
        vk::DescriptorSetLayoutCreateInfo{{}, set_layout_bindings}, nullptr,
        GetVkDispatch());

    assert(descriptor_set.result == vk::Result::eSuccess);

    vk_pipeline_descriptor_set_.emplace_back(descriptor_set.value);

    std::array<vk::DescriptorPoolSize, 1> pool_sizes{};
    pool_sizes[0].type = vk::DescriptorType::eStorageImage;
    pool_sizes[0].descriptorCount = 1;

    vk::DescriptorPoolCreateInfo pool_create_info{{}, 1, pool_sizes};

    auto pool_create_ret = GetVkDevice().createDescriptorPool(
        pool_create_info, nullptr, GetVkDispatch());
    assert(pool_create_ret.result == vk::Result::eSuccess);
    vk_descriptor_pool_ = pool_create_ret.value;
  }

  void CreatePipeline() {
    auto vertex_shader_data = skity::Data::MakeFromFileName(
        EXAMPLE_VK_SHADER_ROOT "vk_demo_buffer_shader_vert.spv");
    auto frag_shader_data = skity::Data::MakeFromFileName(
        EXAMPLE_VK_SHADER_ROOT "vk_demo_buffer_shader_frag.spv");

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
        0, 7 * sizeof(float), vk::VertexInputRate::eVertex};

    std::array<vk::VertexInputAttributeDescription, 3> vertex_input_attr_desc{};
    vertex_input_attr_desc[0].binding = 0;
    vertex_input_attr_desc[0].location = 0;
    vertex_input_attr_desc[0].format = vk::Format::eR32G32Sfloat;
    vertex_input_attr_desc[0].offset = 0;

    vertex_input_attr_desc[1].binding = 0;
    vertex_input_attr_desc[1].location = 1;
    vertex_input_attr_desc[1].format = vk::Format::eR32Sfloat;
    vertex_input_attr_desc[1].offset = 2 * sizeof(float);

    vertex_input_attr_desc[2].binding = 0;
    vertex_input_attr_desc[2].location = 2;
    vertex_input_attr_desc[2].format = vk::Format::eR32G32B32A32Sfloat;
    vertex_input_attr_desc[2].offset = 3 * sizeof(float);

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

    // pipeline layout

    std::vector<vk::PushConstantRange> pipeline_push_constant_range;
    pipeline_push_constant_range.emplace_back(vk::PushConstantRange{
        vk::ShaderStageFlagBits::eFragment, 0, sizeof(glm::vec4)});

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
        {}, vk_pipeline_descriptor_set_, pipeline_push_constant_range};

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
        -0.5f, -0.5f, 1.f,  1.f, 0.f, 1.f, 1.0f,  // vertex 1
        0.5f,  -0.5f, 1.f,  0.f, 1.f, 0.f, 1.0f,  // vertex 2
        -0.5f, 0.5f,  1.f,  0.f, 0.f, 1.f, 1.0f,  // vertex 3
        -0.5f, -0.5f, -1.f, 1.f, 0.f, 1.f, 1.0f,  // vertex 4
        -0.5f, 0.5f,  -1.f, 0.f, 0.f, 1.f, 1.0f,  // vertex 5
        0.5f,  0.5f,  -1.f, 0.f, 1.f, 0.f, 1.0f,  // vertex 6
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

  void CreateStencilImage() {
    auto frameExtend = GetFrameExtend();
    screen_info_.x = (float)frameExtend.width;
    screen_info_.y = (float)frameExtend.height;
    vk::ImageCreateInfo image_create_info{};
    image_create_info.imageType = vk::ImageType::e2D;
    image_create_info.extent.width = frameExtend.width;
    image_create_info.extent.height = frameExtend.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = vk::SampleCountFlagBits::e1;
    image_create_info.tiling = vk::ImageTiling::eOptimal;
    image_create_info.format = vk::Format::eR8G8B8A8Unorm;
    image_create_info.usage =
        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;

    auto create_ret = GetVkDevice().createImage(image_create_info);
    assert(create_ret.result == vk::Result::eSuccess);

    vk_image_ = create_ret.value;

    auto memory_requirements =
        GetVkDevice().getImageMemoryRequirements(vk_image_, GetVkDispatch());
    auto memory_type = FindMemoryType(memory_requirements.memoryTypeBits,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal);

    auto allocate_ret = GetVkDevice().allocateMemory(
        vk::MemoryAllocateInfo{memory_requirements.size, memory_type}, nullptr,
        GetVkDispatch());

    assert(allocate_ret.result == vk::Result::eSuccess);

    vk_image_memory_ = allocate_ret.value;

    GetVkDevice().bindImageMemory(vk_image_, vk_image_memory_, 0,
                                  GetVkDispatch());

    SetImageLayout(vk_image_, vk::ImageAspectFlagBits::eColor,
                   vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

    vk::ImageViewCreateInfo image_view_create_info{};
    image_view_create_info.image = vk_image_;
    image_view_create_info.viewType = vk::ImageViewType::e2D;
    image_view_create_info.format = vk::Format::eR8G8B8A8Unorm;
    image_view_create_info.components.r = vk::ComponentSwizzle::eIdentity;
    image_view_create_info.components.g = vk::ComponentSwizzle::eIdentity;
    image_view_create_info.components.b = vk::ComponentSwizzle::eIdentity;
    image_view_create_info.components.a = vk::ComponentSwizzle::eIdentity;
    image_view_create_info.subresourceRange.aspectMask =
        vk::ImageAspectFlagBits::eColor;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    auto image_view_create_ret = GetVkDevice().createImageView(
        image_view_create_info, nullptr, GetVkDispatch());

    assert(image_view_create_ret.result == vk::Result::eSuccess);

    vk_image_view_ = image_view_create_ret.value;
  }

  void CreateDescriptorSet() {
    vk::DescriptorSetAllocateInfo allocate_info{vk_descriptor_pool_,
                                                vk_pipeline_descriptor_set_};

    auto allocate_ret =
        GetVkDevice().allocateDescriptorSets(allocate_info, GetVkDispatch());

    assert(allocate_ret.result == vk::Result::eSuccess);

    vk_descriptor_set_ = allocate_ret.value.front();

    vk::DescriptorImageInfo image_info{};
    image_info.imageLayout = vk::ImageLayout::eGeneral;
    image_info.imageView = vk_image_view_;

    vk::WriteDescriptorSet descriptor_write{};
    descriptor_write.dstSet = vk_descriptor_set_;
    descriptor_write.descriptorType = vk::DescriptorType::eStorageImage;
    descriptor_write.dstBinding = 0;
    descriptor_write.pImageInfo = &image_info;
    descriptor_write.descriptorCount = 1;

    GetVkDevice().updateDescriptorSets(descriptor_write, {}, GetVkDispatch());
  }

 private:
  glm::vec4 screen_info_;
  std::vector<vk::DescriptorSetLayout> vk_pipeline_descriptor_set_;
  vk::PipelineLayout vk_pipeline_layout_;
  vk::Pipeline vk_pipeline_;
  vk::Buffer vk_vertex_buffer_;
  vk::DeviceMemory vk_vertex_buffer_memory_;
  vk::DescriptorPool vk_descriptor_pool_;
  vk::Image vk_image_;
  vk::DeviceMemory vk_image_memory_;
  vk::ImageView vk_image_view_;
  vk::DescriptorSet vk_descriptor_set_;
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
