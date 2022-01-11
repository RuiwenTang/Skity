#ifndef SKITY_TEXT_TEXT_BLOB_HPP
#define SKITY_TEXT_TEXT_BLOB_HPP

#include <skity/graphic/paint.hpp>
#include <skity/macros.hpp>
#include <skity/text/text_run.hpp>
#include <vector>

namespace skity {

class Typeface;

/**
 * Immutable container which to hold TextRun.
 *
 */
class SK_API TextBlob final {
 public:
  TextBlob(std::vector<TextRun> runs) : text_run_(std::move(runs)) {}
  ~TextBlob() = default;

  std::vector<TextRun> const& getTextRun() const { return text_run_; }

 private:
  std::vector<TextRun> text_run_ = {};
};

class TypefaceDelegate {
 public:
  virtual ~TypefaceDelegate() = default;

  virtual std::shared_ptr<Typeface> fallback(GlyphID glyph_id,
                                             Paint const& text_paint) = 0;

  virtual std::vector<std::vector<GlyphID>> breakTextRun(const char* text) = 0;

  static std::unique_ptr<TypefaceDelegate> CreateSimpleFallbackDelegate(
      std::vector<std::shared_ptr<Typeface>> const& typefaces);
};

class SK_API TextBlobBuilder final {
 public:
  TextBlobBuilder() = default;
  ~TextBlobBuilder() = default;

  std::shared_ptr<TextBlob> buildTextBlob(const char* text, Paint const& paint,
                                          TypefaceDelegate* delegate = nullptr);

 private:
  std::shared_ptr<TextBlob> GenerateBlobWithoutDelegate(const char* text,
                                                        Paint const& paint);

  std::shared_ptr<TextBlob> GenerateBlobWithDelegate(
      const char* text, Paint const& paint, TypefaceDelegate* delegate);

  std::shared_ptr<TextBlob> GenerateBlobWithMultiRun(
      std::vector<std::vector<GlyphID>> const& glyph_ids, Paint const& paint,
      TypefaceDelegate* delegate);

  std::vector<TextRun> GenerateTextRuns(
      std::vector<GlyphID> const& glyphs,
      std::shared_ptr<Typeface> const& typeface, Paint const& paint,
      TypefaceDelegate* delegate);

  TextRun GenerateTextRun(std::vector<GlyphID> const& glyphs,
                          std::shared_ptr<Typeface> const& typeface,
                          float font_size);
};

}  // namespace skity

#endif  // SKITY_TEXT_TEXT_BLOB_HPP