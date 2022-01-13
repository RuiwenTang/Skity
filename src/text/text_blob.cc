#include <cmath>
#include <skity/text/text_blob.hpp>
#include <skity/text/text_run.hpp>
#include <skity/text/typeface.hpp>

namespace skity {

Vec2 TextBlob::getBoundSize() const {
  float height = 0.f;
  float width = 0.f;

  for (auto const &run : text_run_) {
    auto const &glyphs = run.getGlyphInfo();
    for (auto const &glyph : glyphs) {
      height = std::max(glyph.ascent - glyph.descent, height);
      width += glyph.advance_x;
    }
  }

  return Vec2{width, height};
}

class SimpleDelegate : public TypefaceDelegate {
 public:
  SimpleDelegate(std::vector<std::shared_ptr<Typeface>> typeface)
      : typefaces_(std::move(typeface)) {}

  ~SimpleDelegate() override = default;

  std::shared_ptr<Typeface> fallback(GlyphID glyph_id,
                                     Paint const &text_paint) override {
    for (auto const &typeface : typefaces_) {
      if (typeface->containGlyph(glyph_id)) {
        return typeface;
      }
    }

    return nullptr;
  }

  std::vector<std::vector<GlyphID>> breakTextRun(const char *text) override {
    return {};
  }

 private:
  std::vector<std::shared_ptr<Typeface>> typefaces_;
};

std::unique_ptr<TypefaceDelegate>
TypefaceDelegate::CreateSimpleFallbackDelegate(
    std::vector<std::shared_ptr<Typeface>> const &typefaces) {
  if (typefaces.empty()) {
    return std::unique_ptr<TypefaceDelegate>();
  }

  return std::make_unique<SimpleDelegate>(typefaces);
}

std::shared_ptr<TextBlob> TextBlobBuilder::buildTextBlob(
    const char *text, const Paint &paint, TypefaceDelegate *delegate) {
  if (!paint.getTypeface()) {
    return nullptr;
  }

  if (delegate) {
    return GenerateBlobWithDelegate(text, paint, delegate);
  } else {
    return GenerateBlobWithoutDelegate(text, paint);
  }
}

std::shared_ptr<TextBlob> TextBlobBuilder::GenerateBlobWithDelegate(
    const char *text, Paint const &paint, TypefaceDelegate *delegate) {
  auto typeface = paint.getTypeface();

  auto break_result = delegate->breakTextRun(text);

  if (break_result.empty()) {
    std::vector<GlyphID> glyph_id = {};
    typeface->textToGlyphId(text, glyph_id);

    auto runs = GenerateTextRuns(glyph_id, typeface, paint, delegate);

    return std::make_shared<TextBlob>(runs);
  } else {
    return GenerateBlobWithMultiRun(break_result, paint, delegate);
  }
}

std::shared_ptr<TextBlob> TextBlobBuilder::GenerateBlobWithoutDelegate(
    const char *text, Paint const &paint) {
  auto typeface = paint.getTypeface();

  std::vector<GlyphID> glyph_id = {};

  typeface->textToGlyphId(text, glyph_id);

  if (glyph_id.empty()) {
    return nullptr;
  }

  std::vector<TextRun> runs = {};

  runs.emplace_back(GenerateTextRun(glyph_id, typeface, paint.getTextSize(),
                                    paint.getStyle() != Paint::kFill_Style));

  return std::make_shared<TextBlob>(runs);
}

std::shared_ptr<TextBlob> TextBlobBuilder::GenerateBlobWithMultiRun(
    std::vector<std::vector<GlyphID>> const &glyph_ids, Paint const &paint,
    TypefaceDelegate *delegate) {
  auto typeface = paint.getTypeface();

  std::vector<TextRun> runs = {};

  for (auto const &glyphs : glyph_ids) {
    auto sub_runs = GenerateTextRuns(glyphs, typeface, paint, delegate);
    runs.insert(runs.end(), sub_runs.begin(), sub_runs.end());
  }

  return std::make_shared<TextBlob>(runs);
}

std::vector<TextRun> TextBlobBuilder::GenerateTextRuns(
    std::vector<GlyphID> const &glyphs,
    std::shared_ptr<Typeface> const &typeface, Paint const &paint,
    TypefaceDelegate *delegate) {
  float font_size = paint.getTextSize();
  std::vector<TextRun> runs = {};
  auto prev_typeface = typeface;
  auto current_typeface = typeface;

  bool need_path = paint.getStyle() != Paint::kFill_Style;

  std::vector<GlyphInfo> infos = {};
  for (auto glyph_id : glyphs) {
    if (current_typeface->containGlyph(glyph_id)) {
      infos.emplace_back(
          current_typeface->getGlyphInfo(glyph_id, font_size, need_path));
      continue;
    }

    // need to create a new TextRun
    runs.emplace_back(TextRun(current_typeface, std::move(infos), font_size));

    // check if prev_typeface contains this glyph
    if (prev_typeface != current_typeface &&
        prev_typeface->containGlyph(glyph_id)) {
      infos.emplace_back(
          prev_typeface->getGlyphInfo(glyph_id, font_size, need_path));
      current_typeface = prev_typeface;
      continue;
    }

    // fallback to base typeface

    auto fallback_typeface = delegate->fallback(glyph_id, paint);
    if (!fallback_typeface) {
      // failed fallback
      continue;
    }

    prev_typeface = current_typeface;
    current_typeface = fallback_typeface;
    infos.emplace_back(
        current_typeface->getGlyphInfo(glyph_id, font_size, need_path));
  }

  if (!infos.empty()) {
    runs.emplace_back(TextRun(current_typeface, std::move(infos), font_size));
  }

  return runs;
}

TextRun TextBlobBuilder::GenerateTextRun(
    std::vector<GlyphID> const &glyphs,
    std::shared_ptr<Typeface> const &typeface, float font_size,
    bool need_path) {
  std::vector<GlyphInfo> infos = {};

  for (GlyphID id : glyphs) {
    // TODO maybe check if need glyph path
    GlyphInfo info = typeface->getGlyphInfo(id, font_size, need_path);
    if (info.id == 0) {
      continue;
    }

    infos.emplace_back(info);
  }

  return TextRun{typeface, infos, font_size};
}

}  // namespace skity