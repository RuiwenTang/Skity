#include <skity/text/text_run.hpp>
#include <skity/text/typeface.hpp>

namespace skity {

TextRun::TextRun(std::shared_ptr<Typeface> const& typeface,
                 std::vector<GlyphInfo> info, float font_size)
    : typeface_(typeface),
      glyph_info_(std::move(info)),
      font_size_(font_size) {}

TextRun::~TextRun() = default;

GlyphBitmapInfo TextRun::queryBitmapInfo(GlyphID glyph_id) {
  auto typeface = LockTypeface();

  if (!typeface) {
    return {};
  }

  return typeface->getGlyphBitmapInfo(glyph_id, font_size_);
}

}  // namespace skity