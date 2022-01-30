#ifndef SKITY_SRC_RENDER_HW_VK_PIPELINES_STATIC_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_PIPELINES_STATIC_PIPELINE_HPP

#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

namespace skity {

/**
 * @class StaticPipeline
 * @brief Base class for all pipeline which do not need stencil test
 *
 */
class StaticPipeline : public RenderPipeline {
 public:
  StaticPipeline(size_t push_const_size) : RenderPipeline(push_const_size) {}
  ~StaticPipeline() override = default;

 protected:
  VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo()
      override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_PIPELINES_STATIC_PIPELINE_HPP