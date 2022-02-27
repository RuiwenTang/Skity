#include "src/text/ft_library_wrap.hpp"

#include <codecvt>
#include <locale>
#include <skity/io/data.hpp>

#include "src/geometry/math.hpp"
#include "src/logging.hpp"

#include FT_OUTLINE_H

namespace skity {

struct FTOutlineExtractInfo {
  Path* path = nullptr;
  FT_Face ft_face;
  float current_font_size = 0.f;
  float screen_width = 0.f;
  float screen_height = 0.f;
};

// https://www.freetype.org/freetype2/docs/glyphs/glyphs-2.html#section-2
static float FTUnitToPixel(float unit, FTOutlineExtractInfo* info) {
  return (unit * info->current_font_size) / info->ft_face->units_per_EM;
}

static int HandleMoveTo(const FT_Vector* to, void* user) {
  auto* info = static_cast<FTOutlineExtractInfo*>(user);
  info->path->moveTo(FTUnitToPixel(to->x, info), FTUnitToPixel(to->y, info));

  return 0;
}

static int HandleLineTo(const FT_Vector* to, void* user) {
  auto* info = static_cast<FTOutlineExtractInfo*>(user);
  info->path->lineTo(FTUnitToPixel(to->x, info), FTUnitToPixel(to->y, info));

  return 0;
}

static int HandleConicTo(const FT_Vector* control, const FT_Vector* to,
                         void* user) {
  auto* info = static_cast<FTOutlineExtractInfo*>(user);
  info->path->quadTo(FTUnitToPixel(control->x, info),
                     FTUnitToPixel(control->y, info),
                     FTUnitToPixel(to->x, info), FTUnitToPixel(to->y, info));
  return 0;
}

static int HandleCubicTo(const FT_Vector* control1, const FT_Vector* control2,
                         const FT_Vector* to, void* user) {
  auto* info = static_cast<FTOutlineExtractInfo*>(user);
  info->path->cubicTo(
      FTUnitToPixel(control1->x, info), FTUnitToPixel(control1->y, info),
      FTUnitToPixel(control2->x, info), FTUnitToPixel(control2->y, info),
      FTUnitToPixel(to->x, info), FTUnitToPixel(to->y, info));
  return 0;
}

FTLibrary::FTLibrary() : ft_library_(0) {
  FT_Error error = FT_Init_FreeType(&ft_library_);

  if (error) {
    LOG_ERROR("Couldn't initialize the library: FT_Init_FreeType() failed");
  }
}

FTLibrary::~FTLibrary() { FT_Done_FreeType(ft_library_); }

std::unique_ptr<FTTypeFace> FTLibrary::LoadTypeface(const char* file_path) {
  FT_Face ft_face;
  FT_Error error = FT_New_Face(ft_library_, file_path, 0, &ft_face);

  if (error) {
    LOG_ERROR("Couldn't load the font file: FT_New_Face() failed");
    return nullptr;
  }
  FT_Select_Charmap(ft_face, FT_ENCODING_UNICODE);
  return std::make_unique<FTTypeFace>(this, ft_face);
}

std::unique_ptr<FTTypeFace> FTLibrary::LoadTypeface(const Data* data) {
  FT_Face ft_face;
  FT_Error error = FT_New_Memory_Face(
      ft_library_, (const FT_Byte*)data->RawData(), data->Size(), 0, &ft_face);

  if (error) {
    LOG_ERROR("Couldn't load font from memory");
    return nullptr;
  }

  FT_Select_Charmap(ft_face, FT_ENCODING_UNICODE);
  return std::make_unique<FTTypeFace>(this, ft_face);
}

FTTypeFace::~FTTypeFace() { FT_Done_Face(ft_face_); }

std::vector<FTGlyphInfo> FTTypeFace::LoadGlyph(const char* text, float fontSize,
                                               float canvasWidth,
                                               float canvasHeight) {
  current_font_size_ = fontSize;
  current_screen_width_ = canvasWidth;
  current_screen_height_ = canvasHeight;
  std::vector<FTGlyphInfo> infos;

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32_conv;

  auto utf32 = utf32_conv.from_bytes(text);

  for (char32_t c : utf32) {
    FT_UInt index = FT_Get_Char_Index(ft_face_, c);

    FT_Error error =
        FT_Load_Glyph(ft_face_, index, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);

    if (error) {
      LOG_ERROR("failed to load Glyph {:x} at index {}", c, index);
      continue;
    }

    FilpOutline();

    float advance_x = ft_face_->glyph->advance.x * current_font_size_ /
                      ft_face_->units_per_EM;
    Path glyph_path = ExtractOutLine();

    infos.emplace_back(FTGlyphInfo{c, glyph_path, advance_x});
  }

  return infos;
}

FTGlyphInfo FTTypeFace::LoadGlyph(GlyphID glyph_id, float font_size,
                                  bool load_path) {
  FTGlyphInfo info;
  if (!FloatNearlyZero(current_font_size_ - font_size)) {
    current_font_size_ = font_size;
    FT_Set_Pixel_Sizes(ft_face_, 0, font_size);
  }

  info.glyph_id = glyph_id;

  FT_UInt index = FT_Get_Char_Index(ft_face_, glyph_id);
  FT_Error error =
      FT_Load_Glyph(ft_face_, index, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);

  if (error) {
    LOG_ERROR("failed to load Glyph {} at index {}", glyph_id, index);
    info.glyph_id = 0;
    return info;
  }

  FilpOutline();
  info.advance_x =
      ft_face_->glyph->advance.x * current_font_size_ / ft_face_->units_per_EM;
  info.advance_y =
      ft_face_->glyph->advance.y * current_font_size_ / ft_face_->units_per_EM;
  info.width = ft_face_->glyph->metrics.width * current_font_size_ /
               ft_face_->units_per_EM;
  info.height = ft_face_->glyph->metrics.height * current_font_size_ /
                ft_face_->units_per_EM;
  info.bearing_x = ft_face_->glyph->metrics.horiBearingX * current_font_size_ /
                   ft_face_->units_per_EM;
  info.bearing_y = ft_face_->glyph->metrics.horiBearingY * current_font_size_ /
                   ft_face_->units_per_EM;

  if (load_path) {
    info.path = ExtractOutLine();
    info.path_font_size = current_font_size_;
  }

  return info;
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

  FTOutlineExtractInfo outline_info;
  outline_info.path = &path;
  outline_info.ft_face = ft_face_;
  outline_info.current_font_size = current_font_size_;
  outline_info.screen_width = current_screen_width_;
  outline_info.screen_height = current_screen_height_;

  FT_Error error =
      FT_Outline_Decompose(&ft_face_->glyph->outline, &callback, &outline_info);

  if (error) {
    LOG_ERROR("FT_Outline_Decompose failed");
  }

  path.close();

  return path;
}

FTGlyphBitmapInfo FTTypeFace::LoadGlyphBitmap(GlyphID glyph_id,
                                              float font_size) {
  if (!FloatNearlyZero(current_font_size_ - font_size)) {
    current_font_size_ = font_size;
    FT_Set_Pixel_Sizes(ft_face_, 0, font_size);
  }

  FT_UInt c_index = FT_Get_Char_Index(ft_face_, glyph_id);

  if (FT_Load_Glyph(ft_face_, c_index, FT_LOAD_RENDER)) {
    LOG_ERROR("Failed to load glyph id: {}", glyph_id);
    return {};
  }

  FTGlyphBitmapInfo info{};

  info.width = ft_face_->glyph->bitmap.width;
  info.height = ft_face_->glyph->bitmap.rows;
  info.buffer = ft_face_->glyph->bitmap.buffer;

  return info;
}

bool FTTypeFace::containGlyph(GlyphID glyph_id) {
  FT_UInt c_index = FT_Get_Char_Index(ft_face_, glyph_id);

  return c_index != 0;
}

}  // namespace skity
