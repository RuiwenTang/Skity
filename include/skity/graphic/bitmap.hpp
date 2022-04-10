#ifndef SKITY_GRAPHIC_BITMAP_HPP
#define SKITY_GRAPHIC_BITMAP_HPP

#include <cstdint>
#include <skity/graphic/color.hpp>
#include <skity/io/pixmap.hpp>
#include <skity/macros.hpp>

namespace skity {

/**
 * @class Bitmap
 * Describtes a two-dimensional raster pixel array. For now the internal format
 * only support RGBA_32
 *
 * Bitmap can be drawn using Canvas with software raster or set pixel directory
 * by calling **Bitmap::setPixel**, **Bitmap::blendPixel**
 */
class SK_API Bitmap {
 public:
  enum class BlendMode {
    kSrcOver,
  };

  Bitmap();
  Bitmap(uint32_t width, uint32_t height);

  Bitmap(Bitmap const&) = delete;
  Bitmap& operator=(Bitmap const&) = delete;

  ~Bitmap() = default;

  Color getPixel(uint32_t x, uint32_t y);

  void setPixel(uint32_t x, uint32_t y, Color color);

  void setPixel(uint32_t x, uint32_t y, Color4f color);

  void blendPixel(uint32_t x, uint32_t y, Color color,
                  BlendMode blend = BlendMode::kSrcOver);

  void blendPixel(uint32_t x, uint32_t y, Color4f color,
                  BlendMode blend = BlendMode::kSrcOver);

  uint32_t width() const;
  uint32_t height() const;

 private:
  std::shared_ptr<Pixmap> pixmap_;
  uint32_t* pixel_addr_;
};

}  // namespace skity

#endif  // SKITY_GRAPHIC_BITMAP_HPP
