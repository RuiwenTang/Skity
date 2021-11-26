#ifndef SKITY_SRC_RENDER_HW_HW_TEXTURE_HPP
#define SKITY_SRC_RENDER_HW_HW_TEXTURE_HPP

#include <cstdint>

namespace skity {

/**
 * @class Hardware texture interface
 *
 */
class HWTexture {
 public:
  enum class Format {
    kRGB,
    kRGBA,
  };

  enum class Type {
    kColorTexture,
    kDepthTexture,
  };

  virtual ~HWTexture() = default;

  virtual void Init(HWTexture::Type type, HWTexture::Format format) = 0;

  virtual void Bind() = 0;
  virtual void UnBind() = 0;

  virtual uint32_t GetWidth() = 0;
  virtual uint32_t GetHeight() = 0;

  virtual void Resize(uint32_t width, uint32_t height) = 0;

  virtual void UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                          uint32_t height, void* data) = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_TEXTURE_HPP