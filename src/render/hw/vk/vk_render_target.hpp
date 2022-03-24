#ifndef SKITY_SRC_RENDER_HW_VK_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_VK_RENDER_TARGET_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <skity/gpu/gpu_vk_context.hpp>

#include "src/render/hw/hw_render_target.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

struct AllocatedImage;
class VKMemoryAllocator;
class VkRenderer;

class RenderTargetTexture : public VKTexture {
 public:
  RenderTargetTexture(VKInterface* interface, VKMemoryAllocator* allocator,
                      VkRenderer* renderer, GPUVkContext* ctx,
                      VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT)
      : VKTexture(interface, allocator, renderer, ctx, flags) {}

  ~RenderTargetTexture() override = default;

  void PrepareForDraw() override { ChangeImageLayout(VK_IMAGE_LAYOUT_GENERAL); }
};

class FinalRenderTargetTexture : public VKTexture {
 public:
  FinalRenderTargetTexture(
      VKInterface* interface, VKMemoryAllocator* allocator,
      VkRenderer* renderer, GPUVkContext* ctx,
      VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT)
      : VKTexture(interface, allocator, renderer, ctx, flags) {}

  ~FinalRenderTargetTexture() override = default;

  void PrepareForDraw() override {
    if (do_filter_) {
      ChangeImageLayout(VK_IMAGE_LAYOUT_GENERAL);
    } else {
      ChangeImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
  }

  void SetIsDoFilter(bool value) { do_filter_ = value; }

 private:
  bool do_filter_ = false;
};

class VKRenderTarget : public HWRenderTarget, public VkInterfaceClient {
  struct Framebuffer {
    VkRenderPass render_pass = {};
    VkFramebuffer frame_buffer = {};

    Framebuffer(VkRenderPass r) : render_pass(r) {}

    void InitFramebuffer(VKInterface* vk_interface, GPUVkContext* ctx,
                         uint32_t width, uint32_t height,
                         VkImageView color_image, VkImageView stencil_image);

    void Destroy(VKInterface* vk_interface, GPUVkContext* ctx);
  };

 public:
  VKRenderTarget(VKInterface* interface, uint32_t width, uint32_t height,
                 VKMemoryAllocator* allocator, VkRenderer* renderer,
                 GPUVkContext* ctx);

  ~VKRenderTarget() override;

  HWTexture* ColorTexture() override { return &color_texture_; }

  HWTexture* HorizontalTexture() override { return &horizontal_texture_; }

  HWTexture* VerticalTexture() override { return &vertical_texture_; }

  void BindColorTexture() override;

  void BlitColorTexture() override;

  void BindHorizontalTexture() override;

  void BindVerticalTexture() override;

  void Init() override;

  void Destroy() override;

  void StartDraw();

  void EndDraw();

  VkFramebuffer CurrentFramebuffer() const { return current_fbo->frame_buffer; }

  VkCommandBuffer CurrentCMD() const { return vk_cmd_; }

 private:
  void CreateStencilImage();
  void InitSubFramebuffers();
  void DestroySubFramebuffers();

  void BeginCurrentRenderPass();

 private:
  VKMemoryAllocator* allocator_ = {};
  VkRenderer* renderer_ = {};
  GPUVkContext* ctx_ = {};
  std::unique_ptr<AllocatedImage> stencil_image_ = {};
  VkImageView stencil_image_view_ = {};
  RenderTargetTexture color_texture_;
  RenderTargetTexture horizontal_texture_;
  FinalRenderTargetTexture vertical_texture_;
  // used to render to texture
  Framebuffer color_fbo_;

  Framebuffer* current_fbo = nullptr;
  VkCommandBuffer vk_cmd_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_RENDER_TARGET_HPP