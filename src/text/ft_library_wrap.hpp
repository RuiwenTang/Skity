#ifndef SKITY_SRC_RENDER_TEXT_FT_LIBRARY_WRAP_HPP
#define SKITY_SRC_RENDER_TEXT_FT_LIBRARY_WRAP_HPP

#ifdef ENABLE_TEXT_RENDER
#include <ft2build.h>

#include <memory>
#include <skity/graphic/path.hpp>
#include <skity/text/typeface.hpp>
#include <vector>
#include FT_FREETYPE_H

namespace skity {
class Data;
class FTTypeFace;

struct FTGlyphInfo {
  GlyphID glyph_id;
  Path path;
  float path_font_size;
  float advance_x;
  float advance_y;
  float width;
  float height;
  float bearing_x;
  float bearing_y;
};

struct FTGlyphBitmapInfo {
  float width = {};
  float height = {};
  uint8_t* buffer = {};
};

class FTLibrary final {
  friend class FTTypeFace;

 public:
  FTLibrary();
  ~FTLibrary();

  std::unique_ptr<FTTypeFace> LoadTypeface(const char* file_path);
  std::unique_ptr<FTTypeFace> LoadTypeface(const Data* data);

 private:
  FT_Library ft_library_;
};

class FTTypeFace final {
 public:
  FTTypeFace(FTLibrary* ft_library, FT_Face ft_face)
      : ft_library_wrap_(ft_library), ft_face_(ft_face) {}
  ~FTTypeFace();

  std::vector<FTGlyphInfo> LoadGlyph(const char* text, float fontSize,
                                     float canvasWidth, float canvasHeight);
  FTGlyphInfo LoadGlyph(GlyphID glyph_id, float font_size, bool load_path);
  FTGlyphBitmapInfo LoadGlyphBitmap(GlyphID glyph_id, float font_size);

  bool containGlyph(GlyphID glyph_id);

 private:
  void FilpOutline();
  Path ExtractOutLine();

 private:
  FTLibrary* ft_library_wrap_;
  FT_Face ft_face_;
  float current_font_size_ = {};
  float current_screen_width_ = {};
  float current_screen_height_ = {};
};

}  // namespace skity
#endif
#endif  // SKITY_SRC_RENDER_TEXT_FT_LIBRARY_WRAP_HPP