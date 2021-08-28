#include "src/codec/jpeg_codec.hpp"

#include <turbojpeg.h>

#include <iostream>

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

std::shared_ptr<Pixmap> JPEGCodec::Decode() { return nullptr; }

std::shared_ptr<Data> JPEGCodec::Encode() { return nullptr; }

}  // namespace skity
