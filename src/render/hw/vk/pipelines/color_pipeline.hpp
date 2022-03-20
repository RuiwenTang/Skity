#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticColorPipeline : public StaticPipeline {
 public:
  StaticColorPipeline(bool use_gs, size_t push_const_size)
      : StaticPipeline(use_gs, push_const_size) {}
  ~StaticColorPipeline() override = default;

  void UploadUniformColor(ColorInfoSet const& info, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;
};

class StencilDiscardColorPipeline : public StaticColorPipeline {
 public:
  StencilDiscardColorPipeline(bool use_gs, size_t push_const_size)
      : StaticColorPipeline(use_gs, push_const_size) {}

  ~StencilDiscardColorPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilClipColorPipeline : public StaticColorPipeline {
 public:
  StencilClipColorPipeline(bool use_gs, size_t push_const_size)
      : StaticColorPipeline(use_gs, push_const_size) {}

  ~StencilClipColorPipeline() override = default;

  void UpdateStencilInfo(uint32_t reference, uint32_t compare_mask,
                         uint32_t write_mask, GPUVkContext* ctx) override;

 protected:
  std::vector<VkDynamicState> GetDynamicStates() override;
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class StencilKeepColorPipeline : public StaticColorPipeline {
 public:
  StencilKeepColorPipeline(bool use_gs, size_t push_const_size)
      : StaticColorPipeline(use_gs, push_const_size) {}

  ~StencilKeepColorPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

class ColorPipelineFamily : public PipelineFamily {
 public:
  ColorPipelineFamily() = default;
  ~ColorPipelineFamily() override = default;

 protected:
  void OnInit(GPUVkContext* ctx) override;
  void OnAfterInit(GPUVkContext* ctx) override;

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

 private:
  VkShaderModule vs_shader_ = VK_NULL_HANDLE;
  VkShaderModule fs_shader_ = VK_NULL_HANDLE;
  VkShaderModule gs_shader_ = VK_NULL_HANDLE;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP