#include "src/render/text/font_texture.hpp"

#include <functional>

#include "src/geometry/math.hpp"

namespace skity {

FontTexture::FontTexture(Typeface* typeface)
    : TextureAtlas(DEFAULT_SIZE, DEFAULT_SIZE, 1), typeface_(typeface) {}

glm::ivec4 FontTexture::GetGlyphRegion(GlyphID glyph_id, float font_size) {
  GlyphKey key{glyph_id, font_size};

  if (glyph_regions_.count(key) != 0) {
    return glyph_regions_[key];
  }

  return GenerateGlyphRegion(key);
}

glm::ivec4 FontTexture::GenerateGlyphRegion(GlyphKey const& key) {
  // generate text bitmap fron typeface
  auto bitmap_info = typeface_->getGlyphBitmapInfo(key.id, key.font_size);
  // allocate texture region
  glm::ivec4 region = AllocateRegion(bitmap_info.width, bitmap_info.height);

  if (region.x < 0) {
    // allocation falied need resize
    uint32_t width = Width();
    uint32_t height = Height();

    Resize(width * 2.f, height * 2.f);

    region = AllocateRegion(bitmap_info.width, bitmap_info.height);
  }

  // upload text bitmap
  UploadRegion(region.x, region.y, bitmap_info.width, bitmap_info.height,
               bitmap_info.buffer);

  // Fixme to solve black edge for single char
  region.z = bitmap_info.width;
  region.w = bitmap_info.height;
  // save region info
  glyph_regions_.insert(std::make_pair(key, region));

  return region;
}

}  // namespace skity
