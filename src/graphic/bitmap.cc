#include <skity/graphic/bitmap.hpp>
#include <skity/io/data.hpp>

namespace skity {

Bitmap::Bitmap() : pixmap_(), pixel_addr_(nullptr) {}

Bitmap::Bitmap(uint32_t width, uint32_t height)
    : pixmap_(), pixel_addr_(nullptr) {
  size_t total = width * height * 4;

  if (total == 0) {
    return;
  }

  auto data = skity::Data::MakeFromMalloc(std::malloc(total), total);

  if (!data) {
    return;
  }

  pixmap_ = std::make_shared<Pixmap>(data, width * 4, width, height);

  pixel_addr_ = (uint32_t*)pixmap_->Addr();
}

Color Bitmap::getPixel(uint32_t x, uint32_t y) {
  if (!pixmap_) {
    return 0;
  }

  if (x >= width() || y >= height()) {
    return 0;
  }

  return pixel_addr_[y * width() + x];
}

void Bitmap::setPixel(uint32_t x, uint32_t y, Color color) {
  if (!pixmap_) {
    return;
  }

  if (x >= width() || y >= height()) {
    return;
  }

  pixel_addr_[y * width() + x] = color;
}

void Bitmap::setPixel(uint32_t x, uint32_t y, Color4f color) {
  uint8_t a = static_cast<uint8_t>(glm::clamp(color.a * 255, 0.f, 255.f));
  uint8_t r = static_cast<uint8_t>(glm::clamp(color.r * 255, 0.f, 255.f));
  uint8_t g = static_cast<uint8_t>(glm::clamp(color.g * 255, 0.f, 255.f));
  uint8_t b = static_cast<uint8_t>(glm::clamp(color.b * 255, 0.f, 255.f));

  this->setPixel(x, y, ColorSetARGB(a, r, g, b));
}

void Bitmap::blendPixel(uint32_t x, uint32_t y, Color src, BlendMode blend) {
  // For now, only support BlendMode::kSrcOver (ONE, ONE_MINUSE_SRC_ALPHA) blend
  // TODO support other blend mode
  if (!pixmap_) {
    return;
  }

  if (x >= width() || y >= height()) {
    return;
  }

  Color dst = getPixel(x, y);

  float src_alpha = float(ColorGetA(src)) / 255.f;
  float one_minus_alpha = 1.f - src_alpha;

  uint8_t r = ColorGetR(src) * src_alpha +
              static_cast<uint8_t>(ColorGetR(dst) * one_minus_alpha);
  uint8_t g = ColorGetG(src) * src_alpha +
              static_cast<uint8_t>(ColorGetG(dst) * one_minus_alpha);
  uint8_t b = ColorGetB(src) * src_alpha +
              static_cast<uint8_t>(ColorGetB(dst) * one_minus_alpha);
  uint8_t a = ColorGetA(src) * src_alpha +
              static_cast<uint8_t>(ColorGetA(dst) * one_minus_alpha);

  setPixel(x, y, ColorSetARGB(a, r, g, b));
}

void Bitmap::blendPixel(uint32_t x, uint32_t y, Color4f color,
                        BlendMode blend) {
  uint8_t a = static_cast<uint8_t>(glm::clamp(color.a * 255, 0.f, 255.f));
  uint8_t r = static_cast<uint8_t>(glm::clamp(color.r * 255, 0.f, 255.f));
  uint8_t g = static_cast<uint8_t>(glm::clamp(color.g * 255, 0.f, 255.f));
  uint8_t b = static_cast<uint8_t>(glm::clamp(color.b * 255, 0.f, 255.f));

  this->blendPixel(x, y, ColorSetARGB(a, r, g, b), blend);
}

uint32_t Bitmap::width() const {
  if (!pixmap_) {
    return 0;
  }

  return pixmap_->Width();
}

uint32_t Bitmap::height() const {
  if (!pixmap_) {
    return 0;
  }

  return pixmap_->Height();
}

}  // namespace skity
