#ifndef SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP
#define SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP

#include <vulkan/vulkan.h>

#include <memory>

#include "src/render/hw/hw_texture.hpp"

namespace skity {

struct AllocatedBuffer;
struct AllocatedImage;

class VKMemoryAllocator;
class VKPipeline;

class VKTexture : public HWTexture {
 public:
  VKTexture(VKMemoryAllocator* allocator, VKPipeline* pipeline);

  ~VKTexture() override = default;

  void Init(HWTexture::Type type, HWTexture::Format format) override;

  void Bind() override;
  void UnBind() override;

  uint32_t GetWidth() override;
  uint32_t GetHeight() override;

  void Resize(uint32_t width, uint32_t height) override;

  void UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                  uint32_t height, void* data) override;

 private:
  void CreateBufferAndImage();

 private:
  VKMemoryAllocator* allocator_ = {};
  VKPipeline* pipeline_ = {};
  VkFormat format_ = {};
  uint32_t width_ = {};
  uint32_t height_ = {};
  // stage buffer used to hold raw texture data
  std::unique_ptr<AllocatedBuffer> stage_buffer_;
  // allocated vulkan image handler
  std::unique_ptr<AllocatedImage> image_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_VK_VK_TEXTURE_HPP