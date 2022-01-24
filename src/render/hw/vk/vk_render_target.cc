#include "src/render/hw/vk/vk_render_target.hpp"

#include <array>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_pipeline.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

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

  color_fbo_.InitRenderPass(ctx_, color_texture_.GetFormat(), stencil_format);
  color_fbo_.InitFramebuffer(ctx_, Width(), Height(),
                             color_texture_.GetImageView(),
                             stencil_image_view_);

  horizontal_fbo_.InitRenderPass(ctx_, horizontal_texture_.GetFormat(),
                                 stencil_format);
  horizontal_fbo_.InitFramebuffer(ctx_, Width(), Height(),
                                  horizontal_texture_.GetImageView(),
                                  stencil_image_view_);

  vertical_fbo_.InitRenderPass(ctx_, vertical_texture_.GetFormat(),
                               stencil_format);
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
  clear_values[0].color = {0.f, 0.f, 0.f, 0.f};
  clear_values[1].depthStencil = {0.f, 0};

  VkRenderPassBeginInfo render_pass_begin_info{
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  render_pass_begin_info.renderPass = current_fbo->render_pass;
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

void VKRenderTarget::Framebuffer::InitRenderPass(GPUVkContext *ctx,
                                                 VkFormat color_format,
                                                 VkFormat stencil_format) {
  std::array<VkAttachmentDescription, 2> attachments = {};
  // color attachment
  attachments[0].format = color_format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  // depth stencil attachment
  attachments[1].format = ctx->GetDepthStencilFormat();
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_ref;
  color_ref.attachment = 0;
  color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference stencil_ref;
  stencil_ref.attachment = 1;
  stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass_desc{};
  subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass_desc.colorAttachmentCount = 1;
  subpass_desc.pColorAttachments = &color_ref;
  subpass_desc.pDepthStencilAttachment = &stencil_ref;

  VkRenderPassCreateInfo create_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  create_info.attachmentCount = 2;
  create_info.pAttachments = attachments.data();
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass_desc;

  if (VK_CALL(vkCreateRenderPass, ctx->GetDevice(), &create_info, nullptr,
              &render_pass) != VK_SUCCESS) {
    LOG_ERROR("VkRenderTarget can not create render pass!");
  }
}

void VKRenderTarget::Framebuffer::InitFramebuffer(GPUVkContext *ctx,
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

void VKRenderTarget::Framebuffer::Destroy(GPUVkContext *ctx) {
  VK_CALL(vkDestroyFramebuffer, ctx->GetDevice(), frame_buffer, nullptr);
  VK_CALL(vkDestroyRenderPass, ctx->GetDevice(), render_pass, nullptr);
}

}  // namespace skity