#include <skity/io/data.hpp>
#include <skity/io/pixmap.hpp>
#include <utility>

namespace skity {

Pixmap::Pixmap(std::shared_ptr<Data> data, size_t rowBytes, uint32_t width,
               uint32_t height)
    : data_(std::move(data)),
      pixels_(nullptr),
      row_bytes_(rowBytes),
      width_(width),
      height_(height) {
  if (data_) {
    pixels_ = data_->RawData();
  }
}

void Pixmap::Reset() {
  data_ = nullptr;
  pixels_ = nullptr;
  row_bytes_ = 0;
  width_ = 0;
  height_ = 0;
}

}  // namespace skity