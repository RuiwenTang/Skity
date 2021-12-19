#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_IMAGE_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_IMAGE_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticImagePipeline : public StaticPipeline {
 public:
  StaticImagePipeline(size_t push_const_size)
      : StaticPipeline(push_const_size) {}
  ~StaticImagePipeline() override = default;

  void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                          VKFrameBuffer* frame_buffer,
                          VKMemoryAllocator* allocator) override;

  void UploadImageTexture(VKTexture* texture, GPUVkContext* ctx,
                          VKFrameBuffer* frame_buffer,
                          VKMemoryAllocator* allocator) override;

 protected:
  VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) override;

 private:
  glm::vec4 bounds_info_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_IMAGE_PIPELINE_HPP