#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_GRADIENT_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_GRADIENT_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticGradientPipeline : public StaticPipeline {
 public:
  StaticGradientPipeline(size_t push_const_size)
      : StaticPipeline(push_const_size) {}

  ~StaticGradientPipeline() override = default;

  void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                          VKFrameBuffer* frame_buffer,
                          VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_GRADIENT_PIPELINE_HPP