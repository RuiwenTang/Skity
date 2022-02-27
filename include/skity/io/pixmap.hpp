#ifndef SKITY_CODEC_PIXMAP_HPP
#define SKITY_CODEC_PIXMAP_HPP

#include <memory>
#include <skity/macros.hpp>

namespace skity {

class Data;

/**
 * Simple utility class to manage raw pixel data for RGBA pixel format
 *
 */
class SK_API Pixmap final {
 public:
  Pixmap() : data_(), pixels_(nullptr), row_bytes_(0), width_(0), height_(0) {}
  Pixmap(std::shared_ptr<Data> data, size_t rowBytes, uint32_t width,
         uint32_t height);
  ~Pixmap() = default;

  /**
   * Sets width, height, row bytes to zero; pixel address to nullptr
   */
  void Reset();

  size_t RowBytes() const { return row_bytes_; }
  const void* Addr() const { return pixels_; }

  uint32_t Width() const { return width_; }
  uint32_t Height() const { return height_; }

 private:
  // hold this to make sure data not release
  std::shared_ptr<Data> data_;
  const void* pixels_;
  size_t row_bytes_;
  uint32_t width_;
  uint32_t height_;
};

}  // namespace skity

#endif  // SKITY_CODEC_PIXMAP_HPP
