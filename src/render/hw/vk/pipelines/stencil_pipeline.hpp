#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP

#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

namespace skity {

class StencilPipeline : public VKPipelineWrapper {
 public:
  StencilPipeline(size_t push_const_size)
      : VKPipelineWrapper(push_const_size) {}
  ~StencilPipeline() override = default;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;
  VkPipelineColorBlendAttachmentState GetColorBlendState() override;
};

class StencilFrontPipeline : public StencilPipeline {
 public:
  StencilFrontPipeline(size_t push_const_size)
      : StencilPipeline(push_const_size) {}
  ~StencilFrontPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilBackPipeline : public StencilPipeline {
 public:
  StencilBackPipeline(size_t push_const_size)
      : StencilPipeline(push_const_size) {}
  ~StencilBackPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP