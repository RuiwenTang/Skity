#ifndef SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP

#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <skity/gpu/gpu_vk_context.hpp>
#include <vector>

#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

class VKMemoryAllocator;
class SKVkFrameBufferData;
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
  alignas(16) glm::vec4 pos[MAX_COLORS / 4];
};

struct ComputeInfo {
  // [radius, TBD, TBD, TBD]
  alignas(16) glm::vec4 info = {};
  // [BlurType, buffer_width, buffer_height, TBD]
  alignas(16) glm::ivec4 blur_type = {};
  // [ left, top, right, bottom ]
  alignas(16) glm::vec4 bounds = {};
};

class AbsPipelineWrapper {
 public:
  AbsPipelineWrapper() = default;
  virtual ~AbsPipelineWrapper() = default;

  virtual bool IsComputePipeline() = 0;

  virtual bool HasColorSet() = 0;

  virtual void Init(GPUVkContext* ctx, VkShaderModule vertex,
                    VkShaderModule fragment) = 0;

  virtual void Destroy(GPUVkContext* ctx) = 0;

  virtual void Bind(VkCommandBuffer cmd) = 0;

  virtual void Dispatch(VkCommandBuffer cmd, GPUVkContext* ctx) {}

  virtual VkDescriptorSetLayout GetFontSetLayout() { return VK_NULL_HANDLE; }

  virtual void UpdateStencilInfo(uint32_t reference, GPUVkContext* ctx) {}

  virtual void UploadFontSet(VkDescriptorSet set, GPUVkContext* ctx) {}

  virtual void UploadPushConstant(GlobalPushConst const& push_const,
                                  VkCommandBuffer cmd) {}

  virtual void UploadTransformMatrix(glm::mat4 const& matrix, GPUVkContext* ctx,
                                     SKVkFrameBufferData* frame_buffer,
                                     VKMemoryAllocator* allocator) {}

  virtual void UploadCommonSet(CommonFragmentSet const& common_set,
                               GPUVkContext* ctx,
                               SKVkFrameBufferData* frame_buffer,
                               VKMemoryAllocator* allocator) {}

  virtual void UploadUniformColor(ColorInfoSet const& info, GPUVkContext* ctx,
                                  SKVkFrameBufferData* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  virtual void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                                  SKVkFrameBufferData* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  virtual void UploadImageTexture(VKTexture* texture, GPUVkContext* ctx,
                                  SKVkFrameBufferData* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  virtual void UploadBlurInfo(glm::ivec4 const& info, GPUVkContext* ctx,
                              SKVkFrameBufferData* frame_buffer,
                              VKMemoryAllocator* allocator) {}

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticColorPipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilColorPipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilClipColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilKeepColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticGradientPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticGradientPipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper>
  CreateStencilDiscardGradientPipeline(GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper>
  CreateStencilDiscardGradientPipeline(GPUVkContext* ctx,
                                       VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilClipGradientPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilKeepGradientPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticImagePipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilImagePipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilClipImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilKeepImagePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilFrontPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilFrontPipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilClipFrontPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilBackPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilBackPipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilClipBackPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilRecClipBackPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilClipPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilRecClipPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStencilReplacePipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticBlurPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<AbsPipelineWrapper> CreateStaticBlurPipeline(
      GPUVkContext* ctx, VkRenderPass render_pass);

  static std::unique_ptr<AbsPipelineWrapper> CreateComputeBlurPipeline(
      GPUVkContext* ctx);
};

class RenderPipeline : public AbsPipelineWrapper {
 public:
  RenderPipeline(size_t push_const_size) : push_const_size_(push_const_size) {}
  ~RenderPipeline() override = default;

  void SetRenderPass(VkRenderPass render_pass) {
    os_render_pass_ = render_pass;
  }

  bool IsComputePipeline() override { return false; }

  void Init(GPUVkContext* ctx, VkShaderModule vertex,
            VkShaderModule fragment) override;

  void Destroy(GPUVkContext* ctx) override;

  void Bind(VkCommandBuffer cmd) override;

  void UploadPushConstant(GlobalPushConst const& push_const,
                          VkCommandBuffer cmd) override;

  void UploadTransformMatrix(glm::mat4 const& matrix, GPUVkContext* ctx,
                             SKVkFrameBufferData* frame_buffer,
                             VKMemoryAllocator* allocator) override;

  void UploadCommonSet(CommonFragmentSet const& common_set, GPUVkContext* ctx,
                       SKVkFrameBufferData* frame_buffer,
                       VKMemoryAllocator* allocator) override;

  void UploadFontSet(VkDescriptorSet set, GPUVkContext* ctx) override;

  bool HasColorSet() override {
    return descriptor_set_layout_[2] != VK_NULL_HANDLE;
  }

  VkDescriptorSetLayout GetFontSetLayout() override {
    return descriptor_set_layout_[3];
  }

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

  VkCommandBuffer GetBindCMD() { return bind_cmd_; }

  static VkPipelineDepthStencilStateCreateInfo StencilDiscardInfo();
  static VkPipelineDepthStencilStateCreateInfo StencilClipDiscardInfo();
  static VkPipelineDepthStencilStateCreateInfo StencilKeepInfo();

 private:
  VkVertexInputBindingDescription GetVertexInputBinding();
  std::array<VkVertexInputAttributeDescription, 2> GetVertexInputAttributes();

 private:
  size_t push_const_size_;
  VkRenderPass os_render_pass_ = VK_NULL_HANDLE;
  /**
   * set 0: common transform matrix info
   * set 1: common fragment color info, [global alpha, stroke width]
   * set 2: fragment color info
   * set 3: fragment font info
   */
  std::array<VkDescriptorSetLayout, 4> descriptor_set_layout_ = {};
  VkPipelineLayout pipeline_layout_ = {};
  VkPipeline pipeline_ = {};
  VkCommandBuffer bind_cmd_ = {};
};

class ComputePipeline : public AbsPipelineWrapper {
  enum {
    LOCAL_SIZE = 16,
  };

 public:
  ComputePipeline() = default;
  ~ComputePipeline() override = default;

  void Init(GPUVkContext* ctx, VkShaderModule vertex,
            VkShaderModule fragment) override;

  void Destroy(GPUVkContext* ctx) override;

  void Bind(VkCommandBuffer cmd) override;

  void Dispatch(VkCommandBuffer cmd, GPUVkContext* ctx) override;

  void UploadCommonSet(CommonFragmentSet const& common_set, GPUVkContext* ctx,
                       SKVkFrameBufferData* frame_buffer,
                       VKMemoryAllocator* allocator) override;

  void UploadGradientInfo(GradientInfo const& info, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

  void UploadImageTexture(VKTexture* texture, GPUVkContext* ctx,
                          SKVkFrameBufferData* frame_buffer,
                          VKMemoryAllocator* allocator) override;

  bool IsComputePipeline() override { return true; }

  bool HasColorSet() override { return false; }

  void UploadOutputTexture(VKTexture* texture);

 protected:
  glm::vec4 const& CommonInfo() const { return common_info_; }
  glm::vec4 const& BoundsInfo() const { return bounds_info_; }

  VKTexture* InputTexture() const { return input_texture_; }
  VKTexture* OutpuTexture() const { return output_texture_; }

  SKVkFrameBufferData* FrameBufferData() const { return frame_buffer_data_; }
  VKMemoryAllocator* Allocator() const { return allocator_; }

  VkPipelineLayout GetPipelineLayout() { return pipeline_layout_; }

  virtual VkDescriptorSetLayout CreateDescriptorSetLayout(
      GPUVkContext* ctx) = 0;

  virtual void OnDispatch(VkCommandBuffer cmd, GPUVkContext* ctx) = 0;

  VkDescriptorSetLayout ComputeSetLayout() const {
    return descriptor_set_layout_;
  }

 private:
  void UpdateFrameDataAndAllocator(SKVkFrameBufferData* frame_data,
                                   VKMemoryAllocator* allocator) {
    frame_buffer_data_ = frame_data;
    allocator_ = allocator;
  }

  void InitPipelineLayout(GPUVkContext* ctx);

 private:
  glm::vec4 common_info_ = {};
  glm::vec4 bounds_info_ = {};
  VKTexture* input_texture_ = {};
  VKTexture* output_texture_ = {};

  SKVkFrameBufferData* frame_buffer_data_ = {};
  VKMemoryAllocator* allocator_ = {};

  VkCommandBuffer bind_cmd_ = {};
  VkPipelineLayout pipeline_layout_ = {};
  VkDescriptorSetLayout descriptor_set_layout_ = {};
  VkPipeline pipeline_ = {};
};

template <class T>
struct PipelineBuilder {
  const char* vertex_src;
  size_t vertex_size;
  const char* fragment_src;
  size_t fragment_size;
  GPUVkContext* ctx;
  VkRenderPass render_pass;

  PipelineBuilder(const char* vertex_src, size_t vertex_size,
                  const char* fragment_src, size_t fragment_size,
                  GPUVkContext* ctx, VkRenderPass r = VK_NULL_HANDLE)
      : vertex_src(vertex_src),
        vertex_size(vertex_size),
        fragment_src(fragment_src),
        fragment_size(fragment_size),
        ctx(ctx),
        render_pass(r) {}

  std::unique_ptr<T> operator()() {
    auto pipeline = std::make_unique<T>(sizeof(GlobalPushConst));

    if (render_pass) {
      pipeline->SetRenderPass(render_pass);
    }

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