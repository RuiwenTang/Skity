
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <iostream>
#include <string>

extern "C" int MoveToFunc(const FT_Vector* to, void* user) {
  std::cout << "path.moveTo( " << to->x / 64.f << ", " << to->y / 64.f << ");"
            << std::endl;
  return 0;
}

extern "C" int LineToFunc(const FT_Vector* to, void* user) {
  std::cout << "path.lineTo(" << to->x / 64.f << ", " << to->y / 64.f << ");"
            << std::endl;
  return 0;
}

extern "C" int ConicToFunc(const FT_Vector* control, const FT_Vector* to,
                           void* user) {
  std::cout << "path.quadTo(" << control->x / 64.f << ", " << control->y / 64.f
            << " , " << to->x / 64.f << ", " << to->y / 64.f << ");"
            << std::endl;
  return 0;
}

extern "C" int CubicToFunc(const FT_Vector* control1, const FT_Vector* control2,
                           const FT_Vector* to, void* user) {
  std::cout << "path.cubicTo(" << control1->x / 64.f << ", "
            << control1->y / 64.f << ","
            << "" << control2->x / 64.f << ", " << control2->y / 64.f << ","
            << "" << to->x / 64.f << ", " << to->y / 64.f << ");" << std::endl;
  return 0;
}

int main(int argc, const char** argv) {
  std::string path = argv[1];

  FT_Library library;
  FT_Face ft_face;

  auto err = FT_Init_FreeType(&library);
  if (err) {
    std::cerr << "Freetype init error " << std::endl;
    exit(-1);
  }

  err = FT_New_Face(library, path.c_str(), 0, &ft_face);
  if (err) {
    std::cerr << "Freetype load ttf failed" << std::endl;
    exit(-1);
  }

  // set font size
  err = FT_Set_Pixel_Sizes(ft_face, 0, 256);
  if (err) {
    std::cerr << "Freetype set char size failed" << std::endl;
    exit(-1);
  }

  auto glyph_index = FT_Get_Char_Index(ft_face, 'b');

  err = FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_NO_BITMAP);
  if (err) {
    std::cerr << "Freetype load glyph failed" << std::endl;
    exit(-1);
  }

  FT_GlyphSlot slot = ft_face->glyph;

  const FT_Fixed multiplier = 65536L;

  FT_Matrix matrix;

  matrix.xx = 1L * multiplier;
  matrix.xy = 0L * multiplier;
  matrix.yx = 0L * multiplier;
  matrix.yy = -1L * multiplier;

  FT_Outline_Transform(&slot->outline, &matrix);

  FT_Outline_Funcs callbacks;
  callbacks.move_to = &MoveToFunc;
  callbacks.line_to = &LineToFunc;
  callbacks.conic_to = &ConicToFunc;
  callbacks.cubic_to = &CubicToFunc;
  callbacks.shift = 0;
  callbacks.delta = 0;

  err = FT_Outline_Decompose(&slot->outline, &callbacks, nullptr);
  if (err) {
    std::cerr << "decompose outline error" << std::endl;
    exit(-1);
  }

  return 0;
}