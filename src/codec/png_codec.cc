#include "src/codec/png_codec.hpp"

#include <cstring>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>

namespace skity {

#ifdef SKITY_HAS_PNG

#define PNG_BYTES_TO_CHECK 4

PNGCodec::PNGCodec() = default;

PNGCodec::~PNGCodec() { png_image_free(&image_); }

std::shared_ptr<Pixmap> skity::PNGCodec::Decode() {
  if (image_.opaque) {
    png_image_free(&image_);
    image_.opaque = nullptr;
  }
  image_.version = PNG_IMAGE_VERSION;
  if (!png_image_begin_read_from_memory(&image_, data_->RawData(),
                                        data_->Size())) {
    return nullptr;
  }

  png_bytep buffer;
  image_.format = PNG_FORMAT_RGBA;
  size_t raw_data_size = PNG_IMAGE_SIZE(image_);
  buffer = static_cast<png_bytep>(std::malloc(raw_data_size));
  if (!buffer) {
    // out of memory
    png_image_free(&image_);
    return nullptr;
  }

  if (!png_image_finish_read(&image_, nullptr, buffer, 0, nullptr)) {
    std::free(buffer);
    return nullptr;
  }

  auto raw_data = Data::MakeFromMalloc(buffer, raw_data_size);
  uint32_t width = image_.width;
  uint32_t height = image_.height;

  pixmap_ = std::make_shared<Pixmap>(raw_data, width * 4, width, height);

  return pixmap_;
}
std::shared_ptr<Data> skity::PNGCodec::Encode() { return nullptr; }

bool skity::PNGCodec::RecognizeFileType(const char *header, size_t size) {
  return !png_sig_cmp((png_const_bytep)header, (png_size_t)0,
                      PNG_BYTES_TO_CHECK);
}

#endif  // SKITY_HAS_PNG

}  // namespace skity
