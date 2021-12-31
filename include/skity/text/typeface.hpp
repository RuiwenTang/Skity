#ifndef SKITY_TEXT_TYPEFACE_HPP
#define SKITY_TEXT_TYPEFACE_HPP

#include <functional>
#include <memory>
#include <skity/graphic/path.hpp>
#include <skity/macros.hpp>

namespace skity {

class Data;

using GlyphID = uint32_t;

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
 * @class Typeface
 *  Helper class to holding Font information used in paint to draw text.
 */
class SK_API Typeface {
 public:
  ~Typeface();

  /**
   * Create a new typeface given a file. If the file does not exist, or is not a
   * valid font file, returns nullptr.
   * @param path path to font file.
   * @return Typeface instance or nullptr.
   */
  static std::shared_ptr<Typeface> MakeFromFile(const char* path);

  /**
   * Create a new typeface from memory
   * @param data
   * @return
   */
  static std::shared_ptr<Typeface> MakeFromData(
      std::shared_ptr<Data> const& data);

  void textToGlyphId(const char* text, std::vector<GlyphID>& glyphs);

  void textToGlyphInfo(const char* text, float font_size,
                       std::vector<GlyphInfo>& info);

  void getGlyphInfo(std::vector<GlyphID> const& glyph_id, float font_size,
                    std::vector<GlyphInfo>& info, bool load_path = false);

  GlyphInfo getGlyphInfo(GlyphID glyph_id, float font_size,
                         bool load_path = false);

  GlyphBitmapInfo getGlyphBitmapInfo(GlyphID glyph_id, float font_size);

 private:
  class Impl;
  Typeface() = default;

 private:
  std::unique_ptr<Impl, std::function<void(Impl*)>> impl_;
  std::shared_ptr<skity::Data> data_ = {};
};

}  // namespace skity

#endif  // SKITY_TEXT_TYPEFACE_HPP
