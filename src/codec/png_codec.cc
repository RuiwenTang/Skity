#include "src/codec/png_codec.hpp"

#include <cstring>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>
#include <vector>

namespace skity {

#ifdef SKITY_HAS_PNG

#define PNG_BYTES_TO_CHECK 4

static void png_write_callback(png_structp png_ptr, png_bytep data,
                               png_size_t length) {
  png_get_io_ptr(png_ptr);
}

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

struct PNGDestructor {
  png_structp p;
  explicit PNGDestructor(png_structp p) : p(p) {}
  ~PNGDestructor() {
    if (p) {
      png_destroy_write_struct(&p, nullptr);
    }
  }
};

std::shared_ptr<Data> skity::PNGCodec::Encode(const Pixmap* pixmap) {
  if (!pixmap) {
    return nullptr;
  }
  
  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png_ptr) {
    return nullptr;
  }

  PNGDestructor png_destructor{png_ptr};

  info_ptr = png_create_info_struct(png_ptr);
  png_set_IHDR(png_ptr, info_ptr, pixmap->Width(), pixmap->Height(), 8,
               PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_set_rows(png_ptr, info_ptr, (png_bytepp)pixmap->Addr());

  std::vector<uint8_t> encode_data{};
  png_set_write_fn(png_ptr, &encode_data, png_write_callback, nullptr);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

  return Data::MakeWithCopy(encode_data.data(), encode_data.size());
}

bool skity::PNGCodec::RecognizeFileType(const char* header, size_t size) {
  return !png_sig_cmp((png_const_bytep)header, (png_size_t)0,
                      PNG_BYTES_TO_CHECK);
}

#endif  // SKITY_HAS_PNG

}  // namespace skity
