#include "src/codec/jpeg_codec.hpp"

#include <turbojpeg.h>

#include <iostream>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>

struct TJHandlerWrapper {
  TJHandlerWrapper(tjhandle h) : handle(h) {}

  ~TJHandlerWrapper() {
    if (this->handle) {
      tjDestroy(this->handle);
    }
  }

  tjhandle handle = nullptr;
};

namespace skity {

bool JPEGCodec::RecognizeFileType(const char* header, size_t size) {
  TJHandlerWrapper hw{tjInitDecompress()};

  if (!hw.handle) {
    // JPEG init failed
    return false;
  }

  int32_t width;
  int32_t height;

  int ret = tjDecompressHeader(hw.handle, (unsigned char*)header, size, &width,
                               &height);
  if (ret == 0) {
    return true;
  } else {
    char* err = tjGetErrorStr2(hw.handle);
    std::cerr << " jpeg decode header failed : [ " << err << "]" << std::endl;
    return false;
  }
}

std::shared_ptr<Pixmap> JPEGCodec::Decode() {
  TJHandlerWrapper hw{tjInitDecompress()};

  if (!hw.handle) {
    // JPEG init failed
    return false;
  }

  int32_t width;
  int32_t height;

  int ret = tjDecompressHeader(hw.handle, (unsigned char*)data_->RawData(),
                               data_->Size(), &width, &height);

  if (ret != 0) {
    char* err = tjGetErrorStr2(hw.handle);
    std::cerr << " jpeg decode header failed : [ " << err << "]" << std::endl;
    return nullptr;
  }

  uint8_t* buffer = (uint8_t*)tjAlloc(width * height * tjPixelSize[TJPF_RGBA]);

  ret = tjDecompress2(hw.handle, (const unsigned char*)data_->RawData(),
                      data_->Size(), (unsigned char*)buffer, width, 0, height,
                      TJPF_RGBA, 0);
  if (ret != 0) {
    tjFree(buffer);
    char* err = tjGetErrorStr2(hw.handle);
    std::cerr << " jpeg decode image failed : [ " << err << "]" << std::endl;
    return nullptr;
  }

  auto image_data = skity::Data::MakeWithCopy(
      buffer, width * height * tjPixelSize[TJPF_RGBA]);

  tjFree(buffer);
  return std::make_shared<Pixmap>(image_data, width * tjPixelSize[TJPF_RGBA],
                                  width, height);
}

std::shared_ptr<Data> JPEGCodec::Encode() { return nullptr; }

}  // namespace skity
