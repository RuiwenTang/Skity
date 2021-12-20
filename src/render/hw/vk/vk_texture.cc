#include "src/render/hw/vk/vk_texture.hpp"

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_pipeline.hpp"

namespace skity {

static VkFormat hw_texture_format_to_vk_format(HWTexture::Format format) {
  switch (format) {
    case HWTexture::Format::kRGB:
      return VK_FORMAT_R8G8B8_SRGB;
    case HWTexture::Format::kRGBA:
      return VK_FORMAT_R8G8B8A8_SRGB;
    case HWTexture::Format::kR:
      return VK_FORMAT_R8_UINT;
  }
}

static uint32_t vk_format_comp(VkFormat format) {
  if (format == VK_FORMAT_R8G8B8_SRGB) {
    return 3;
  } else if (format == VK_FORMAT_R8G8B8A8_SRGB) {
    return 4;
  } else {
    return 1;
  }
}

VKTexture::VKTexture(VKMemoryAllocator* allocator, VKPipeline* pipeline,
                     GPUVkContext* ctx)
    : HWTexture(), allocator_(allocator), pipeline_(pipeline), ctx_(ctx) {}

void VKTexture::Init(HWTexture::Type type, HWTexture::Format format) {
  this->format_ = hw_texture_format_to_vk_format(format);
  this->bpp_ = vk_format_comp(this->format_);
  this->range_.aspectMask = type == HWTexture::Type::kColorTexture
                                ? VK_IMAGE_ASPECT_COLOR_BIT
                                : VK_IMAGE_ASPECT_DEPTH_BIT;
  this->range_.baseMipLevel = 0;
  this->range_.levelCount = 1;
  this->range_.baseArrayLayer = 0;
  this->range_.layerCount = 1;
}

void VKTexture::Destroy() {
  if (image_) {
    allocator_->FreeImage(image_.get());
  }

  if (vk_image_view_) {
    VK_CALL(vkDestroyImageView, ctx_->GetDevice(), vk_image_view_, nullptr);
  }
}

void VKTexture::Bind() {}

void VKTexture::UnBind() {}

uint32_t VKTexture::GetWidth() { return width_; }

uint32_t VKTexture::GetHeight() { return height_; }

void VKTexture::Resize(uint32_t width, uint32_t height) {
  bool need_create = width != width_ || height != height_;

  width_ = width;
  height_ = height;

  if (need_create) {
    if (image_) {
      allocator_->FreeImage(image_.get());
    }

    if (vk_image_view_) {
      VK_CALL(vkDestroyImageView, ctx_->GetDevice(), vk_image_view_, nullptr);
      vk_image_view_ = VK_NULL_HANDLE;
    }

    CreateBufferAndImage();
  }
}

void VKTexture::UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                           uint32_t height, void* data) {
  size_t buffer_size = width * height * bpp_;
  // step 1 upload data to stage buffer
  std::unique_ptr<AllocatedBuffer> stage_buffer{
      allocator_->AllocateStageBuffer(buffer_size)};

  allocator_->UploadBuffer(stage_buffer.get(), data, buffer_size);

  VkCommandBuffer cmd = pipeline_->ObtainInternalCMD();
  // step 2 set vk_image layout for
  if (image_->GetCurrentLayout() != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    allocator_->TransferImageLayout(cmd, image_.get(), range_,
                                    image_->GetCurrentLayout(),
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  }
  // step 3 transfer image data from stage buffer to image buffer
  VkBufferImageCopy copy_region = {};
  copy_region.bufferOffset = 0;
  copy_region.bufferRowLength = width;
  copy_region.bufferImageHeight = height;

  copy_region.imageSubresource.aspectMask = range_.aspectMask;
  copy_region.imageSubresource.mipLevel = 0;
  copy_region.imageSubresource.baseArrayLayer = 0;
  copy_region.imageSubresource.layerCount = 1;
  copy_region.imageOffset = {(int32_t)offset_x, (int32_t)offset_y, 0};
  copy_region.imageExtent = image_->GetImageExtent();

  // copy the buffer into the image
  allocator_->CopyBufferToImage(cmd, stage_buffer.get(), image_.get(),
                                copy_region);

  pipeline_->SubmitCMD(cmd);

  allocator_->FreeBuffer(stage_buffer.get());
}

void VKTexture::PrepareForDraw() {
  if (image_->GetCurrentLayout() == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    // Texture is ready to draw nothing need todo
    return;
  }

  // transfer image layout to `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`
  VkCommandBuffer cmd = pipeline_->ObtainInternalCMD();

  allocator_->TransferImageLayout(cmd, image_.get(), range_,
                                  image_->GetCurrentLayout(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  pipeline_->SubmitCMD(cmd);
}

VkSampler VKTexture::GetSampler() const { return pipeline_->PipelineSampler(); }

VkImageLayout VKTexture::GetImageLayout() const {
  return image_->GetCurrentLayout();
}

void VKTexture::CreateBufferAndImage() {
  size_t total_size = width_ * height_ * bpp_;

  if (total_size == 0) {
    LOG_ERROR("Create Image buffer failed, the image size is zero!");
    return;
  }

  image_.reset(allocator_->AllocateImage(
      format_, {width_, height_, 1},
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));

  // create image view
  if (!vk_image_view_) {
    auto imageview_create_info = VKUtils::ImageViewCreateInfo(
        image_->GetImageFormat(), image_->GetImage(), range_);

    VK_CALL(vkCreateImageView, ctx_->GetDevice(), &imageview_create_info,
            nullptr, &vk_image_view_);
  }
}

uint32_t VKTexture::BytesPerRow() const { return bpp_ * width_; }

}  // namespace skity