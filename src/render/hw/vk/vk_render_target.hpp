#ifndef SKITY_SRC_RENDER_HW_VK_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_VK_RENDER_TARGET_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <skity/gpu/gpu_context.hpp>

#include "src/render/hw/hw_render_target.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

struct AllocatedImage;
class VKMemoryAllocator;
class SKVkPipelineImpl;

class VKRenderTarget : public HWRenderTarget {
  struct Framebuffer {
    VkRenderPass render_pass = {};
    VkFramebuffer frame_buffer = {};

    void InitRenderPass(GPUVkContext* ctx, VkFormat color_format,
                        VkFormat stencil_format);
    void InitFramebuffer(GPUVkContext* ctx, uint32_t width, uint32_t height,
                         VkImageView color_image, VkImageView stencil_image);

    void Destroy(GPUVkContext* ctx);
  };

 public:
  VKRenderTarget(uint32_t width, uint32_t height, VKMemoryAllocator* allocator,
                 SKVkPipelineImpl* pipeline, GPUVkContext* ctx)
      : HWRenderTarget(width, height),
        allocator_(allocator),
        pipeline_(pipeline),
        ctx_(ctx),
        color_texture_(allocator, pipeline, ctx, true),
        horizontal_texture_(allocator, pipeline, ctx, true),
        vertical_texture_(allocator, pipeline, ctx, true) {}

  ~VKRenderTarget() override;

  HWTexture* ColorTexture() override { return &color_texture_; }

  HWTexture* HorizontalTexture() override { return &horizontal_texture_; }

  HWTexture* VerticalTexture() override { return &vertical_texture_; }

  void BindColorTexture() override;

  void BindHorizontalTexture() override;

  void BindVerticalTexture() override;

  void Init() override;

  void Destroy() override;

  void StartDraw();

  void EndDraw();

  VkFramebuffer CurrentFramebuffer() const { return current_fbo->frame_buffer; }

  VkRenderPass CurrentRenderPass() const { return current_fbo->render_pass; }

  VkCommandBuffer CurrentCMD() const { return vk_cmd_; }

 private:
  void CreateStencilImage();
  void InitSubFramebuffers();
  void DestroySubFramebuffers();

  void BeginCurrentRenderPass();

 private:
  VKMemoryAllocator* allocator_ = {};
  SKVkPipelineImpl* pipeline_ = {};
  GPUVkContext* ctx_ = {};
  std::unique_ptr<AllocatedImage> stencil_image_ = {};
  VkImageView stencil_image_view_ = {};
  VKTexture color_texture_;
  VKTexture horizontal_texture_;
  VKTexture vertical_texture_;
  // used to render to texture
  Framebuffer color_fbo_ = {};
  Framebuffer horizontal_fbo_ = {};
  Framebuffer vertical_fbo_ = {};

  Framebuffer* current_fbo = nullptr;
  VkCommandBuffer vk_cmd_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_RENDER_TARGET_HPP