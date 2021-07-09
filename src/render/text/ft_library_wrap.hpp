#ifndef SKITY_SRC_RENDER_TEXT_FT_LIBRARY_WRAP_HPP
#define SKITY_SRC_RENDER_TEXT_FT_LIBRARY_WRAP_HPP

#include <ft2build.h>

#include <memory>
#include <skity/graphic/path.hpp>
#include <vector>
#include FT_FREETYPE_H

namespace skity {

class FTTypeFace;

class FTLibrary final {
  friend class FTTypeFace;

 public:
  FTLibrary();
  ~FTLibrary();

  std::unique_ptr<FTTypeFace> LoadTypeface(const char* file_path);

 private:
  FT_Library ft_library_;
};

class FTTypeFace final {
 public:
  FTTypeFace(FTLibrary* ft_library, FT_Face ft_face)
      : ft_library_wrap_(ft_library), ft_face_(ft_face) {}
  ~FTTypeFace();

  std::vector<Path> LoadGlyph(const char* text);

 private:
  void FilpOutline();
  Path ExtractOutLine();

 private:
  FTLibrary* ft_library_wrap_;
  FT_Face ft_face_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_TEXT_FT_LIBRARY_WRAP_HPP