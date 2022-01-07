#ifndef SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_HPP

#include <map>
#include <skity/gpu/gpu_context.hpp>

#include "src/render/hw/hw_pipeline.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_pipeline_wrapper.hpp"

namespace skity {

class VKTexture;
class VKFontTexture;

template <class T>
struct DirtyValueHolder {
  T value = {};
  bool dirty = true;
};

class SKVkPipelineImpl : public HWPipeline {
 public:
  SKVkPipelineImpl(GPUVkContext* ctx);
  ~SKVkPipelineImpl() override;

  void Init() override;

  void Destroy() override;

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

  // internal vulkan helper functions
  VKMemoryAllocator* Allocator() const { return vk_memory_allocator_.get(); }

  VkCommandBuffer ObtainInternalCMD();

  void SubmitCMD(VkCommandBuffer cmd);

  void WaitForFence();

  void ResetFence();

  VkFence PipelineFence() const { return vk_fence_; }
  VkSampler PipelineSampler() const { return vk_sampler_; }

 private:
  void InitCMDPool();
  void InitFence();
  void InitSampler();
  void InitFrameBuffers();
  void InitPipelines();
  void InitVertexBuffer(size_t new_size);
  void InitIndexBuffer(size_t new_size);

  void DestroyCMDPool();
  void DestroyFence();
  void DestroySampler();
  void DestroyPipelines();
  void DestroyFrameBuffers();

  VKPipelineWrapper* PickColorPipeline();
  VKPipelineWrapper* PickStencilPipeline();
  VKPipelineWrapper* PickGradientPipeline();
  VKPipelineWrapper* PickImagePipeline();

  void BindPipelineIfNeed(VKPipelineWrapper* pipeline);

  void UpdatePushConstantIfNeed(VKPipelineWrapper* pipeline);
  void UpdateTransformMatrixIfNeed(VKPipelineWrapper* pipeline);
  void UpdateCommonSetIfNeed(VKPipelineWrapper* pipeline);
  void UpdateStencilConfigIfNeed(VKPipelineWrapper* pipeline);
  void UpdateColorInfoIfNeed(VKPipelineWrapper* pipeline);
  void UpdateFontInfoIfNeed(VKPipelineWrapper* pipeline);

  SKVkFrameBufferData* CurrentFrameBuffer();

 private:
  GPUVkContext* ctx_ = {};
  VkCommandPool vk_cmd_pool_ = {};
  VkFence vk_fence_ = {};
  VkSampler vk_sampler_ = {};
  HWPipelineColorMode color_mode_ = HWPipelineColorMode::kUniformColor;
  HWStencilFunc stencil_func_ = HWStencilFunc::ALWAYS;
  HWStencilOp stencil_op_ = HWStencilOp::KEEP;
  bool enable_stencil_test_ = false;
  bool enable_color_output_ = true;
  uint8_t stencil_write_mask_ = 0xFF;
  uint8_t stencil_compare_mask_ = 0xFF;
  uint8_t stencil_value_ = 0;
  std::unique_ptr<VKMemoryAllocator> vk_memory_allocator_ = {};
  std::vector<std::unique_ptr<SKVkFrameBufferData>> frame_buffer_ = {};
  // used to check if need to bind pipeline
  VKPipelineWrapper* prev_pipeline_ = nullptr;

  // color pipelines
  std::unique_ptr<VKPipelineWrapper> static_color_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_color_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_clip_color_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_keep_color_pipeline_ = {};
  // gradient pipelines
  std::unique_ptr<VKPipelineWrapper> static_gradient_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_gradient_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_clip_gradient_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_keep_gradient_pipeline_ = {};
  // image pipelines
  std::unique_ptr<VKPipelineWrapper> static_image_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_image_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_clip_image_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_keep_image_pipeline_ = {};
  // stencil pipelines
  std::unique_ptr<VKPipelineWrapper> stencil_front_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_clip_front_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_back_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_clip_back_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_rec_clip_back_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_clip_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_rec_clip_pipeline_ = {};
  std::unique_ptr<VKPipelineWrapper> stencil_replace_pipeline_ = {};

  std::unique_ptr<AllocatedBuffer> vertex_buffer_ = {};
  std::unique_ptr<AllocatedBuffer> index_buffer_ = {};
  DirtyValueHolder<GlobalPushConst> global_push_const_ = {};
  DirtyValueHolder<glm::mat4> model_matrix_ = {};
  DirtyValueHolder<CommonFragmentSet> common_fragment_set_ = {};
  DirtyValueHolder<ColorInfoSet> color_info_set_ = {};
  DirtyValueHolder<GradientInfo> gradient_info_set_ = {};

  VKTexture* image_texture_ = nullptr;
  VKTexture* font_texture_ = nullptr;
  VkDescriptorSet empty_font_set_ = VK_NULL_HANDLE;
  std::unique_ptr<VKFontTexture> empty_font_texture_ = {};
  std::map<VKTexture*, VkDescriptorSet> used_font_and_set_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_HPP