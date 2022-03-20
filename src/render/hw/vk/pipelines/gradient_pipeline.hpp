#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_GRADIENT_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_GRADIENT_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticGradientPipeline : public StaticPipeline {
 public:
  StaticGradientPipeline(bool use_gs, size_t push_const_size)
      : StaticPipeline(use_gs, push_const_size) {}

  ~StaticGradientPipeline() override = default;

  void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;
};

class StencilDiscardGradientPipeline : public StaticGradientPipeline {
 public:
  StencilDiscardGradientPipeline(bool use_gs, size_t push_const_size)
      : StaticGradientPipeline(use_gs, push_const_size) {}
  ~StencilDiscardGradientPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilClipGradientPipeline : public StaticGradientPipeline {
 public:
  StencilClipGradientPipeline(bool use_gs, size_t push_const_size)
      : StaticGradientPipeline(use_gs, push_const_size) {}

  ~StencilClipGradientPipeline() override = default;

  void UpdateStencilInfo(uint32_t reference, uint32_t compare_mask,
                         uint32_t write_mask, GPUVkContext* ctx) override;

 protected:
  std::vector<VkDynamicState> GetDynamicStates() override;
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilKeepGradientPipeline : public StaticGradientPipeline {
 public:
  StencilKeepGradientPipeline(bool use_gs, size_t push_const_size)
      : StaticGradientPipeline(use_gs, push_const_size) {}
  ~StencilKeepGradientPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class GradientPipelineFamily : public PipelineFamily {
 public:
  GradientPipelineFamily() = default;
  ~GradientPipelineFamily() override = default;

 protected:
  std::tuple<const char*, size_t> GetFragmentShaderInfo() override;

  std::unique_ptr<AbsPipelineWrapper> CreateStaticPipeline(
      GPUVkContext* ctx) override;
  std::unique_ptr<AbsPipelineWrapper> CreateStencilDiscardPipeline(
      GPUVkContext* ctx) override;
  std::unique_ptr<AbsPipelineWrapper> CreateStencilClipPipeline(
      GPUVkContext* ctx) override;
  std::unique_ptr<AbsPipelineWrapper> CreateStencilKeepPipeline(
      GPUVkContext* ctx) override;
  std::unique_ptr<AbsPipelineWrapper> CreateOSStaticPipeline(
      GPUVkContext* ctx) override;
  std::unique_ptr<AbsPipelineWrapper> CreateOSStencilPipeline(
      GPUVkContext* ctx) override;

};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_GRADIENT_PIPELINE_HPP