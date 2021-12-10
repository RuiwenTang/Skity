#ifndef SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_HPP

#include <skity/gpu/gpu_context.hpp>

#include "src/render/hw/hw_pipeline.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

namespace skity {

class VKPipeline : public HWPipeline {
 public:
  VKPipeline(GPUVkContext* ctx);
  ~VKPipeline() override;

  void Init() override;

  void Bind() override;

  void UnBind() override;

  void SetViewProjectionMatrix(glm::mat4 const& mvp) override;

  void SetModelMatrix(glm::mat4 const& matrix) override;

  void SetPipelineColorMode(HWPipelineColorMode mode) override;

  void SetStrokeWidth(float width) override;

  void SetUniformColor(glm::vec4 const& color) override;

  void SetGradientBoundInfo(glm::vec4 const& info) override;

  void SetGradientCountInfo(int32_t color_count, int32_t pos_count) override;

  void SetGradientColors(std::vector<Color4f> const& colors) override;

  void SetGradientPositions(std::vector<float> const& pos) override;

  void UploadVertexBuffer(void* data, size_t data_size) override;

  void UploadIndexBuffer(void* data, size_t data_size) override;

  void SetGlobalAlpha(float alpha) override;

  void EnableStencilTest() override;

  void DisableStencilTest() override;

  void EnableColorOutput() override;

  void DisableColorOutput() override;

  void UpdateStencilMask(uint8_t write_mask) override;

  void UpdateStencilOp(HWStencilOp op) override;

  void UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                         uint32_t compare_mask) override;

  void DrawIndex(uint32_t start, uint32_t count) override;

  void BindTexture(HWTexture* texture, uint32_t slot) override;

 private:
  void InitPipelines();

 private:
  GPUVkContext* ctx_;
  std::unique_ptr<VKMemoryAllocator> vk_memory_allocator_ = {};
  std::unique_ptr<VKPipelineWrapper> static_color_pipeline_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_HPP