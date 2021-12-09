#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP

#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

namespace skity {

class ColorPipeline : public VKPipelineWrapper {
 public:
  ColorPipeline(size_t push_const_size);
  ~ColorPipeline() override = default;

 protected:
  std::vector<VkDescriptorSetLayout> GenearteDescriptorSetLayout(
      GPUVkContext* ctx) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP