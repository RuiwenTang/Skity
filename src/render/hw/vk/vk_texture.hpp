#ifndef SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP
#define SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <skity/gpu/gpu_vk_context.hpp>

#include "src/render/hw/hw_texture.hpp"

namespace skity {

struct AllocatedBuffer;
struct AllocatedImage;

class VKMemoryAllocator;
class VkRenderer;

class VKTexture : public HWTexture {
 public:
  VKTexture(VKMemoryAllocator* allocator, VkRenderer* renderer,
            GPUVkContext* ctx,
            VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT);

  ~VKTexture() override = default;

  void Init(HWTexture::Type type, HWTexture::Format format) override;

  void Destroy() override;

  void Bind() override;
  void UnBind() override;

  uint32_t GetWidth() override;
  uint32_t GetHeight() override;

  void Resize(uint32_t width, uint32_t height) override;

  void UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                  uint32_t height, void* data) override;

  virtual void PrepareForDraw();

  void ChangeImageLayout(VkImageLayout target_layout);

  void ChangeImageLayoutWithoutSumbit(VkImageLayout target_layout);

  VkSampler GetSampler() const;

  VkImageView GetImageView() const { return vk_image_view_; }

  VkImage GetImage() const;

  VkImageLayout GetImageLayout() const;

  VkFormat GetFormat() const { return format_; }

  AllocatedImage* GetAllocatedImage() const { return image_.get(); }

  VkImageSubresourceRange const& Range() const { return range_; }

 private:
  void CreateBufferAndImage();

  uint32_t BytesPerRow() const;

 private:
  VKMemoryAllocator* allocator_ = {};
  VkRenderer* renderer_ = {};
  GPUVkContext* ctx_ = {};
  VkImageUsageFlags flags_ = {};
  VkFormat format_ = {};
  VkImageSubresourceRange range_ = {};
  VkImageView vk_image_view_ = {};
  uint32_t bpp_ = {};
  uint32_t width_ = {};
  uint32_t height_ = {};
  // allocated vulkan image handler
  std::unique_ptr<AllocatedImage> image_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP