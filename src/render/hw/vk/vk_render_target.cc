#include "src/render/hw/vk/vk_render_target.hpp"

#include <array>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_pipeline.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

VKRenderTarget::VKRenderTarget(uint32_t width, uint32_t height,
                               VKMemoryAllocator* allocator,
                               SKVkPipelineImpl* pipeline, GPUVkContext* ctx)
    : HWRenderTarget(width, height),
      allocator_(allocator),
      pipeline_(pipeline),
      ctx_(ctx),
      color_texture_(allocator, pipeline, ctx, true),
      horizontal_texture_(allocator, pipeline, ctx, true),
      vertical_texture_(allocator, pipeline, ctx, true),
      color_fbo_(pipeline->OffScreenRenderPass()),
      horizontal_fbo_(pipeline->OffScreenRenderPass()),
      vertical_fbo_(pipeline->OffScreenRenderPass()) {}

VKRenderTarget::~VKRenderTarget() = default;

void VKRenderTarget::BindColorTexture() {
  current_fbo = &color_fbo_;
  BeginCurrentRenderPass();
}

void VKRenderTarget::BindHorizontalTexture() {
  VK_CALL(vkCmdEndRenderPass, vk_cmd_);

  current_fbo = &horizontal_fbo_;
  BeginCurrentRenderPass();
}

void VKRenderTarget::BindVerticalTexture() {
  VK_CALL(vkCmdEndRenderPass, vk_cmd_);

  current_fbo = &vertical_fbo_;
  BeginCurrentRenderPass();
}

void VKRenderTarget::Init() {
  // create all images
  CreateStencilImage();

  color_texture_.Init(HWTexture::Type::kColorTexture, HWTexture::Format::kRGBA);
  color_texture_.Resize(Width(), Height());

  horizontal_texture_.Init(HWTexture::Type::kColorTexture,
                           HWTexture::Format::kRGBA);
  horizontal_texture_.Resize(Width(), Height());

  vertical_texture_.Init(HWTexture::Type::kColorTexture,
                         HWTexture::Format::kRGBA);
  vertical_texture_.Resize(Width(), Height());

  InitSubFramebuffers();
}

void VKRenderTarget::Destroy() {
  DestroySubFramebuffers();

  allocator_->FreeImage(stencil_image_.get());
  VK_CALL(vkDestroyImageView, ctx_->GetDevice(), stencil_image_view_, nullptr);

  color_texture_.Destroy();
  horizontal_texture_.Destroy();
  vertical_texture_.Destroy();
}

void VKRenderTarget::StartDraw() {
  if (vk_cmd_ != nullptr) {
    LOG_ERROR("VKRenderTarget: state error, previouse cmd not submit");
  }

  vk_cmd_ = pipeline_->ObtainInternalCMD();
}

void VKRenderTarget::EndDraw() {
  VK_CALL(vkCmdEndRenderPass, vk_cmd_);

  pipeline_->SubmitCMD(vk_cmd_);

  vk_cmd_ = VK_NULL_HANDLE;
}

void VKRenderTarget::CreateStencilImage() {
  stencil_image_.reset(allocator_->AllocateImage(
      ctx_->GetDepthStencilFormat(), {Width(), Height(), 1},
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));

  VkImageSubresourceRange range;
  range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  range.baseMipLevel = 0;
  range.levelCount = 1;
  range.baseArrayLayer = 0;
  range.layerCount = 1;

  auto create_info = VKUtils::ImageViewCreateInfo(
      ctx_->GetDepthStencilFormat(), stencil_image_->GetImage(), range);

  VK_CALL(vkCreateImageView, ctx_->GetDevice(), &create_info, nullptr,
          &stencil_image_view_);
}

void VKRenderTarget::InitSubFramebuffers() {
  VkFormat stencil_format = ctx_->GetDepthStencilFormat();

  color_fbo_.InitFramebuffer(ctx_, Width(), Height(),
                             color_texture_.GetImageView(),
                             stencil_image_view_);

  horizontal_fbo_.InitFramebuffer(ctx_, Width(), Height(),
                                  horizontal_texture_.GetImageView(),
                                  stencil_image_view_);

  vertical_fbo_.InitFramebuffer(ctx_, Width(), Height(),
                                vertical_texture_.GetImageView(),
                                stencil_image_view_);
}

void VKRenderTarget::DestroySubFramebuffers() {
  color_fbo_.Destroy(ctx_);
  horizontal_fbo_.Destroy(ctx_);
  vertical_fbo_.Destroy(ctx_);
}

void VKRenderTarget::BeginCurrentRenderPass() {
  std::array<VkClearValue, 2> clear_values{};
  clear_values[0].color = {1.f, 0.f, 0.f, 1.f};
  clear_values[1].depthStencil = {0.f, 0};

  VkRenderPassBeginInfo render_pass_begin_info{
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  render_pass_begin_info.renderPass = pipeline_->OffScreenRenderPass();
  render_pass_begin_info.framebuffer = current_fbo->frame_buffer;
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent = {Width(), Height()};
  render_pass_begin_info.clearValueCount = clear_values.size();
  render_pass_begin_info.pClearValues = clear_values.data();

  VK_CALL(vkCmdBeginRenderPass, vk_cmd_, &render_pass_begin_info,
          VK_SUBPASS_CONTENTS_INLINE);

  // view port
  VkViewport view_port{
      0,   0,  static_cast<float>(Width()), static_cast<float>(Height()),
      0.f, 1.f};

  VK_CALL(vkCmdSetViewport, vk_cmd_, 0, 1, &view_port);
  // scissor
  VkRect2D scissor{{0, 0}, VkExtent2D{Width(), Height()}};

  VK_CALL(vkCmdSetScissor, vk_cmd_, 0, 1, &scissor);
}

void VKRenderTarget::Framebuffer::InitFramebuffer(GPUVkContext* ctx,
                                                  uint32_t width,
                                                  uint32_t height,
                                                  VkImageView color_image,
                                                  VkImageView stencil_image) {
  std::array<VkImageView, 2> attachments = {};
  attachments[0] = color_image;
  attachments[1] = stencil_image;

  VkFramebufferCreateInfo create_info{
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};

  create_info.attachmentCount = 2;
  create_info.pAttachments = attachments.data();
  create_info.renderPass = render_pass;
  create_info.width = width;
  create_info.height = height;
  create_info.layers = 1;

  if (VK_CALL(vkCreateFramebuffer, ctx->GetDevice(), &create_info, nullptr,
              &frame_buffer) != VK_SUCCESS) {
    LOG_ERROR("VkRenderTarget can not create framebuffer!");
  }
}

void VKRenderTarget::Framebuffer::Destroy(GPUVkContext* ctx) {
  VK_CALL(vkDestroyFramebuffer, ctx->GetDevice(), frame_buffer, nullptr);
}

}  // namespace skity