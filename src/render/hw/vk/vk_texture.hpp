#ifndef SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP
#define SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <skity/gpu/gpu_context.hpp>

#include "src/render/hw/hw_texture.hpp"

namespace skity {

struct AllocatedBuffer;
struct AllocatedImage;

class VKMemoryAllocator;
class VKPipeline;

class VKTexture : public HWTexture {
 public:
  VKTexture(VKMemoryAllocator* allocator, VKPipeline* pipeline,
            GPUVkContext* ctx);

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

  void PrepareForDraw();

  VkSampler GetSampler() const;

  VkImageView GetImageView() const { return vk_image_view_; }

  VkImageLayout GetImageLayout() const;

 private:
  void CreateBufferAndImage();

  uint32_t BytesPerRow() const;

 private:
  VKMemoryAllocator* allocator_ = {};
  VKPipeline* pipeline_ = {};
  GPUVkContext* ctx_ = {};
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