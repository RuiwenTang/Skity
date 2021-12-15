#ifndef SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP

#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <skity/gpu/gpu_context.hpp>

namespace skity {

class VKMemoryAllocator;
class VKFrameBuffer;

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

class VKPipelineWrapper {
 public:
  VKPipelineWrapper(size_t push_const_size)
      : push_const_size_(push_const_size) {}
  virtual ~VKPipelineWrapper() = default;

  void Init(GPUVkContext* ctx, VkShaderModule vertex, VkShaderModule fragment);

  void Destroy(GPUVkContext* ctx);

  void Bind(VkCommandBuffer cmd);

  void UploadPushConstant(GlobalPushConst const& push_const,
                          VkCommandBuffer cmd);

  void UploadTransformMatrix(glm::mat4 const& matrix, GPUVkContext* ctx,
                             VKFrameBuffer* frame_buffer,
                             VKMemoryAllocator* allocator);

  void UploadCommonSet(CommonFragmentSet const& common_set, GPUVkContext* ctx,
                       VKFrameBuffer* frame_buffer,
                       VKMemoryAllocator* allocator);

  bool HasColorSet() { return descriptor_set_layout_[2] != VK_NULL_HANDLE; }

  // sub class implement
  virtual void UploadUniformColor(ColorInfoSet const& info, GPUVkContext* ctx,
                                  VKFrameBuffer* frame_buffer,
                                  VKMemoryAllocator* allocator) {}

  static std::unique_ptr<VKPipelineWrapper> CreateStaticColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilColorPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilFrontPipeline(
      GPUVkContext* ctx);

  static std::unique_ptr<VKPipelineWrapper> CreateStencilBackPipeline(
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

 private:
  VkVertexInputBindingDescription GetVertexInputBinding();
  std::array<VkVertexInputAttributeDescription, 2> GetVertexInputAttributes();

 private:
  size_t push_const_size_;
  /**
   * set 0: common transform matrix info
   * set 1: common fragment color info, [global alpha, stroke width]
   * set 2: fragment color info
   */
  std::array<VkDescriptorSetLayout, 3> descriptor_set_layout_ = {};
  VkPipelineLayout pipeline_layout_ = {};
  VkPipeline pipeline_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_PIPELINE_WRAPPER_HPP