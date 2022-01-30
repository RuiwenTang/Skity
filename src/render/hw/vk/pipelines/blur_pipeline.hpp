#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_BLUR_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_BLUR_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticBlurPipeline : public StaticPipeline {
 public:
  StaticBlurPipeline(size_t push_const_size)
      : StaticPipeline(push_const_size) {}
  ~StaticBlurPipeline() override = default;

  void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

  void UploadImageTexture(VKTexture* texture, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

  void UploadBlurInfo(glm::ivec4 const& info, GPUVkContext* ctx,
                      SKVkFrameBufferData* frame_buffer,
                      VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;

 private:
  glm::vec4 bounds_info_ = {};
  glm::ivec4 blur_info_ = {};
};

class FinalBlurPipeline : public StaticBlurPipeline {
 public:
  FinalBlurPipeline(size_t push_const_size)
      : StaticBlurPipeline(push_const_size) {}
  ~FinalBlurPipeline() override = default;

 protected:
  VkPipelineColorBlendAttachmentState GetColorBlendState() override;
};

class ComputeBlurPipeline : public ComputePipeline {
 public:
  ComputeBlurPipeline() = default;
  ~ComputeBlurPipeline() override = default;

  void UploadBlurInfo(glm::ivec4 const& info, GPUVkContext* ctx,
                      SKVkFrameBufferData* frame_buffer,
                      VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout CreateDescriptorSetLayout(GPUVkContext* ctx) override;

  void OnDispatch(VkCommandBuffer cmd, GPUVkContext* ctx) override;

 private:
  glm::ivec4 blur_info_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_BLUR_PIPELINE_HPP