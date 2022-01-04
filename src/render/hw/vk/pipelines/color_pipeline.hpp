#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticColorPipeline : public StaticPipeline {
 public:
  StaticColorPipeline(size_t push_const_size)
      : StaticPipeline(push_const_size) {}
  ~StaticColorPipeline() override = default;

  void UploadUniformColor(ColorInfoSet const& info, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;
};

class StencilDiscardColorPipeline : public StaticColorPipeline {
 public:
  StencilDiscardColorPipeline(size_t push_const_size)
      : StaticColorPipeline(push_const_size) {}

  ~StencilDiscardColorPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilClipColorPipeline : public StaticColorPipeline {
 public:
  StencilClipColorPipeline(size_t push_const_size)
      : StaticColorPipeline(push_const_size) {}

  ~StencilClipColorPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilKeepColorPipeline : public StaticColorPipeline {
 public:
  StencilKeepColorPipeline(size_t push_const_size)
      : StaticColorPipeline(push_const_size) {}

  ~StencilKeepColorPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP