#include "src/render/hw/vk/vk_font_texture.hpp"

#include "src/render/hw/vk/vk_memory.hpp"

namespace skity {

VKFontTexture::VKFontTexture(VKInterface* interface, Typeface* typeface,
                             VKMemoryAllocator* allocator, VkRenderer* renderer,
                             GPUVkContext* ctx)
    : HWFontTexture(typeface), VKTexture(interface, allocator, renderer, ctx) {}

VKFontTexture::~VKFontTexture() = default;

void VKFontTexture::Init() {
  VKTexture::Init(HWTexture::Type::kColorTexture, HWTexture::Format::kR);

  this->VKTexture::Resize(Width(), Height());
}

void VKFontTexture::Destroy() { VKTexture::Destroy(); }

HWTexture* VKFontTexture::GetHWTexture() { return this; }

void VKFontTexture::OnUploadRegion(uint32_t x, uint32_t y, uint32_t width,
                                   uint32_t height, uint8_t* data) {
  this->UploadData(x, y, width, height, data);
}

void VKFontTexture::OnResize(uint32_t new_width, uint32_t new_height) {
  this->VKTexture::Resize(new_width, new_width);
}

}  // namespace skity