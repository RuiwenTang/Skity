#ifndef SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP

#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <skity/gpu/gpu_context.hpp>
#include <vector>

#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

class VKMemoryAllocator;
class VKFrameBuffer;
class VKTexture;

struct GlobalPushConst {
  alignas(16) glm::mat4 mvp = {};
  alignas(16) int32_t premul_alpha = {};
};

struct CommonFragmentSet {
  // [global_alpha, stroke_width, TBD, TBD]
  alignas(16) glm::vec4 info = {};
};

struct ColorInfoSet {
  alignas(16) glm::vec4 user_color = {};
};

struct GradientInfo {
  enum {
    MAX_COLORS = 32,
  };

  alignas(16) glm::ivec4 count = {};
  alignas(16) glm::vec4 bounds = {};
  alignas(16) glm::vec4 colors[MAX_COLORS];
  alignas(16) float pos[MAX_COLORS];
};

class VKPipelineWrapper {
 public:
  VKPipelineWrapper(size_t push_const_size)
      : push_const_size_(push_const_size) {}
  virtual ~VKPipelineWrapper() = default;

  void Init(GPUVkContext* ctx, VkShaderModule vertex, VkShaderModule fragment);

  void Destroy(GPUVkContext* ctx);

  void Bind(VkCommandBuffer cmd);

  virtual void UpdateStencilInfo(uint32_t reference, GPUVkContext* ctx) {}

  void UploadPushConstant(GlobalPushConst const& push_const,
                          VkCommandBuffer cmd);

  void UploadTransformMatrix(glm::mat4 const& matrix, GPUVkContext* ctx,
                             VKFrameBuffer* frame_buffer,
                             VKMemoryAllocator* allocator);

  void UploadCommonSet(CommonFragmentSet const& common_set, GPUVkContext* ctx,
                       VKFrameBuffer* frame_buffer,
                       VKMemoryAllocator* allocator);

  void UploadFontSet(VkDescriptorSet set, GPUVkContext* ctx);

  bool HasColorSet() { return descriptor_set_layout_[2] != VK_NULL_HANDLE; }

  VkDescriptorSetLayout GetFontSetLayout() { return descriptor_set_layout_[3]; }

  // sub class implement
  virtual void UploadUniformColor(ColorInfoSet const& info, GPUVkContext* ctx,
                                  VKFrameBuffer* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  // sub class implement
  virtual void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                                  VKFrameBuffer* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  // sub class implement
  virtual void UploadImageTexture(VKTexture* texture, GPUVkContext* ctx,
                                  VKFrameBuffer* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  static std::unique_ptr<VKPipelineWrapper> CreateStaticColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilClipColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilKeepColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStaticGradientPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper>
  CreateStencilDiscardGradientPipeline(GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilClipGradientPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilKeepGradientPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStaticImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilClipImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilKeepImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilFrontPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilBackPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilClipPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilReplacePipeline(
      GPUVkContext* ctx);

 protected:
  void InitDescriptorSetLayout(GPUVkContext* ctx);
  void InitPipelineLayout(GPUVkContext* ctx);

  virtual VkDescriptorSetLayout GenerateColorSetLayout(GPUVkContext* ctx) = 0;

  virtual VkPipelineDepthStencilStateCreateInfo
  GetDepthStencilStateCreateInfo() = 0;

  virtual VkPipelineColorBlendAttachmentState GetColorBlendState();

  virtual std::vector<VkDynamicState> GetDynamicStates();

  VkDescriptorSetLayout GetColorSetLayout() {
    return descriptor_set_layout_[2];
  }

  VkPipelineLayout GetPipelineLayout() { return pipeline_layout_; }

  static VkPipelineDepthStencilStateCreateInfo StencilDiscardInfo();
  static VkPipelineDepthStencilStateCreateInfo StencilClipDiscardInfo();
  static VkPipelineDepthStencilStateCreateInfo StencilKeepInfo();

 private:
  VkVertexInputBindingDescription GetVertexInputBinding();
  std::array<VkVertexInputAttributeDescription, 2> GetVertexInputAttributes();

 private:
  size_t push_const_size_;
  /**
   * set 0: common transform matrix info
   * set 1: common fragment color info, [global alpha, stroke width]
   * set 2: fragment color info
   * set 3: fragment font info
   */
  std::array<VkDescriptorSetLayout, 4> descriptor_set_layout_ = {};
  VkPipelineLayout pipeline_layout_ = {};
  VkPipeline pipeline_ = {};
};

template <class T>
struct PipelineBuilder {
  const char* vertex_src;
  size_t vertex_size;
  const char* fragment_src;
  size_t fragment_size;
  GPUVkContext* ctx;

  PipelineBuilder(const char* vertex_src, size_t vertex_size,
                  const char* fragment_src, size_t fragment_size,
                  GPUVkContext* ctx)
      : vertex_src(vertex_src),
        vertex_size(vertex_size),
        fragment_src(fragment_src),
        fragment_size(fragment_size),
        ctx(ctx) {}

  std::unique_ptr<T> operator()() {
    auto pipeline = std::make_unique<T>(sizeof(GlobalPushConst));

    auto vertex = VKUtils::CreateShader(ctx->GetDevice(),
                                        (const char*)vertex_src, vertex_size);

    auto fragment = VKUtils::CreateShader(
        ctx->GetDevice(), (const char*)fragment_src, fragment_size);
    pipeline->Init(ctx, vertex, fragment);

    VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vertex, nullptr);
    VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fragment, nullptr);

    return pipeline;
  }
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP