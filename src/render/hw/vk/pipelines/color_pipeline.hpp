#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP

#include "src/render/hw/vk/pipelines/static_pipeline.hpp"

namespace skity {

class StaticColorPipeline : public StaticPipeline {
 public:
  StaticColorPipeline(size_t push_const_size)
      : StaticPipeline(push_const_size) {}
  ~StaticColorPipeline() override = default;

 protected:
  std::vector<VkDescriptorSetLayout> GenearteDescriptorSetLayout(
      GPUVkContext* ctx) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_COLOR_PIPELINE_HPP