#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP

#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

namespace skity {

class StencilPipeline : public RenderPipeline {
 public:
  StencilPipeline(bool use_gs, size_t push_const_size)
      : RenderPipeline(use_gs, push_const_size) {}
  ~StencilPipeline() override = default;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;
  VkPipelineColorBlendAttachmentState GetColorBlendState() override;
};

class StencilFrontPipeline : public StencilPipeline {
 public:
  StencilFrontPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}
  ~StencilFrontPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilClipFrontPipeline : public StencilPipeline {
 public:
  StencilClipFrontPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}

  ~StencilClipFrontPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilBackPipeline : public StencilPipeline {
 public:
  StencilBackPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}
  ~StencilBackPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilClipBackPipeline : public StencilPipeline {
 public:
  StencilClipBackPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}

  ~StencilClipBackPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilRecursiveClipBackPipeline : public StencilPipeline {
 public:
  StencilRecursiveClipBackPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}

  ~StencilRecursiveClipBackPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

// Used for move stencil buffer for clip
class StencilClipPipeline : public StencilPipeline {
 public:
  StencilClipPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}
  ~StencilClipPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilRecursiveClipPipeline : public StencilPipeline {
 public:
  StencilRecursiveClipPipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}
  ~StencilRecursiveClipPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

// Used for clip buffer clear or convex polygon clip
class StencilReplacePipeline : public StencilPipeline {
 public:
  StencilReplacePipeline(bool use_gs, size_t push_const_size)
      : StencilPipeline(use_gs, push_const_size) {}
  ~StencilReplacePipeline() override = default;

  void UpdateStencilInfo(uint32_t reference, uint32_t compare_mask,
                         uint32_t write_mask, GPUVkContext* ctx) override;

 protected:
  std::vector<VkDynamicState> GetDynamicStates() override;
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_STENCIL_PIPELINE_HPP