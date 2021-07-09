#include "src/render/text/ft_library_wrap.hpp"

#include <codecvt>
#include <iostream>
#include <string>

#include FT_OUTLINE_H

namespace skity {

static int HandleMoveTo(const FT_Vector* to, void* user) {
  Path* path = static_cast<Path*>(user);
  path->moveTo(to->x, to->y);

  return 0;
}

static int HandleLineTo(const FT_Vector* to, void* user) {
  Path* path = static_cast<Path*>(user);
  path->lineTo(to->x, to->y);

  return 0;
}

static int HandleConicTo(const FT_Vector* control, const FT_Vector* to,
                         void* user) {
  Path* path = static_cast<Path*>(user);
  path->quadTo(control->x, control->y, to->x, to->y);
  return 0;
}

static int HandleCubicTo(const FT_Vector* control1, const FT_Vector* control2,
                         const FT_Vector* to, void* user) {
  Path* path = static_cast<Path*>(user);
  path->cubicTo(control1->x, control1->y, control2->x, control2->y, to->x,
                to->y);
  return 0;
}

FTLibrary::FTLibrary() : ft_library_(0) {
  FT_Error error = FT_Init_FreeType(&ft_library_);

  if (error) {
    std::cerr << "Couldn't initialize the library: FT_Init_FreeType() failed"
              << std::endl;
  }
}

FTLibrary::~FTLibrary() { FT_Done_FreeType(ft_library_); }

std::unique_ptr<FTTypeFace> FTLibrary::LoadTypeface(const char* file_path) {
  FT_Face ft_face;
  FT_Error error = FT_New_Face(ft_library_, file_path, 0, &ft_face);

  if (error) {
    std::cerr << "Couldn't load the font file: FT_New_Face() failed"
              << std::endl;
    return nullptr;
  }

  return std::make_unique<FTTypeFace>(this, ft_face);
}

FTTypeFace::~FTTypeFace() { FT_Done_Face(ft_face_); }

std::vector<Path> FTTypeFace::LoadGlyph(const char* text) {
  std::vector<Path> paths;

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32_conv;

  auto utf32 = utf32_conv.from_bytes(text);

  for (char32_t c : utf32) {
    FT_UInt index = FT_Get_Char_Index(ft_face_, c);

    FT_Error error =
        FT_Load_Glyph(ft_face_, index, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);

    if (error) {
      std::cerr << "failed to load Glyph " << std::endl;
      continue;
    }

    FilpOutline();
    Path glyph_path = ExtractOutLine();

    // glyph_path.dump();

    paths.emplace_back(glyph_path);
  }

  return paths;
}

void FTTypeFace::FilpOutline() {
  const FT_Fixed multiplier = 65536L;

  FT_Matrix matrix;
  matrix.xx = 1L * multiplier;
  matrix.xy = 0L * multiplier;
  matrix.yx = 0L * multiplier;
  matrix.yy = -1L * multiplier;

  FT_Outline_Transform(&ft_face_->glyph->outline, &matrix);
}

Path FTTypeFace::ExtractOutLine() {
  Path path;

  FT_Outline_Funcs callback;
  callback.move_to = HandleMoveTo;
  callback.line_to = HandleLineTo;
  callback.conic_to = HandleConicTo;
  callback.cubic_to = HandleCubicTo;

  callback.shift = 0;
  callback.delta = 0;

  FT_Error error =
      FT_Outline_Decompose(&ft_face_->glyph->outline, &callback, &path);

  if (error) {
    std::cerr << "FT_Outline_Decompose failed" << std::endl;
  }

  return path;
}

}  // namespace skity