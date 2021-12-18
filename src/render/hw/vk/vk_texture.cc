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

VKTexture::VKTexture(VKMemoryAllocator* allocator, VKPipeline* pipeline)
    : HWTexture(), allocator_(allocator), pipeline_(pipeline) {}

void VKTexture::Init(HWTexture::Type type, HWTexture::Format format) {
  this->format_ = hw_texture_format_to_vk_format(format);
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
    if (stage_buffer_) {
      allocator_->FreeBuffer(stage_buffer_.get());
    }

    if (image_) {
      allocator_->FreeImage(image_.get());
    }

    CreateBufferAndImage();
  }
}

void VKTexture::UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                           uint32_t height, void* data) {}

void VKTexture::CreateBufferAndImage() {
  size_t total_size = width_ * height_ * vk_format_comp(format_);

  if (total_size == 0) {
    LOG_ERROR("Create Image buffer failed, the image size is zero!");
    return;
  }

  stage_buffer_.reset(allocator_->AllocateStageBuffer(total_size));

  image_.reset(allocator_->AllocateImage(
      format_, {width_, height_, 1},
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
}

}  // namespace skity