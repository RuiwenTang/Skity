#ifndef SKITY_TEXT_TEXT_RUN_HPP
#define SKITY_TEXT_TEXT_RUN_HPP

#include <memory>
#include <skity/graphic/path.hpp>
#include <skity/macros.hpp>
#include <vector>

namespace skity {

using GlyphID = uint32_t;

class Typeface;

struct SK_API GlyphInfo {
  GlyphID id;
  Path path;
  float advance_x;
  float advance_y;
  float ascent;
  float descent;
  float width;
  float height;
  float font_size;
  float bearing_x;
};

struct SK_API GlyphBitmapInfo {
  float width = {};
  float height = {};
  uint8_t* buffer = {};
};

/**
 * Simple class for represents a sequence of characters that share a single
 * property set.
 * Like `Typeface`, `FontSize` ...
 *
 */
class SK_API TextRun final {
 public:
  TextRun(std::shared_ptr<Typeface> const& typeface,
          std::vector<GlyphInfo> info, float font_size);
  ~TextRun();

  std::vector<GlyphInfo> const& getGlyphInfo() const;

  GlyphBitmapInfo queryBitmapInfo(GlyphID glyph_id);

 private:
  std::shared_ptr<Typeface> LockTypeface() { return typeface_.lock(); }

 private:
  std::weak_ptr<Typeface> typeface_ = {};
  std::vector<GlyphInfo> glyph_info_;
  float font_size_ = 0.f;
};

}  // namespace skity

#endif  // SKITY_TEXT_TEXT_RUN_HPP