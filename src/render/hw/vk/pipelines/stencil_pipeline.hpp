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

// Used for move stencil buffer for clip
class StencilClipPipeline : public StencilPipeline {
 public:
  StencilClipPipeline(size_t push_const_size)
      : StencilPipeline(push_const_size) {}
  ~StencilClipPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

// Used for clip buffer clear or convex polygon clip
class StencilReplacePipeline : public StencilPipeline {
 public:
  StencilReplacePipeline(size_t push_const_size)
      : StencilPipeline(push_const_size) {}
  ~StencilReplacePipeline() override = default;

  void UpdateStencilInfo(uint32_t reference, GPUVkContext* ctx) override;

 protected:
  std::vector<VkDynamicState> GetDynamicStates() override;
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP