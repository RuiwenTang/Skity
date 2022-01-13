#include <gtest/gtest.h>

#include <skity/skity.hpp>

#include "test_config.hpp"

TEST(TextBlobBuilder, test_simple_build) {
  skity::TextBlobBuilder builder;

  const char* text = "and all the men who came to";

  auto typeface = skity::Typeface::MakeFromFile(TEST_BUILD_IN_FONT);

  skity::Paint paint;
  paint.setTextSize(15.f);
  paint.setTypeface(typeface);

  auto text_blob = builder.buildTextBlob(text, paint);

  EXPECT_TRUE(text_blob != nullptr);

  auto runs = text_blob->getTextRun();

  EXPECT_FALSE(runs.empty());
  EXPECT_EQ(runs.size(), 1);

  auto const& glyphs = runs.front().getGlyphInfo();

  EXPECT_EQ(glyphs.size(), 27);
}

TEST(TextBlobBuilder, test_simple_fallback) {
  skity::TextBlobBuilder builder;

  const char* text = "the aid of the party.🎉";

  auto typeface = skity::Typeface::MakeFromFile(TEST_BUILD_IN_FONT);

  skity::Paint paint;
  paint.setTextSize(15.f);
  paint.setTypeface(typeface);

  std::vector<std::shared_ptr<skity::Typeface>> fallbacks = {};
  fallbacks.emplace_back(
      skity::Typeface::MakeFromFile(TEST_BUILD_IN_EMOJI_FONT));

  auto delegate =
      skity::TypefaceDelegate::CreateSimpleFallbackDelegate(fallbacks);

  auto text_blob = builder.buildTextBlob(text, paint, delegate.get());

  auto const& runs = text_blob->getTextRun();

  EXPECT_EQ(runs.size(), 2);

  auto glyphs1 = runs[0].getGlyphInfo();
  auto glyphs2 = runs[1].getGlyphInfo();

  EXPECT_EQ(glyphs1.size(), 21);
  EXPECT_EQ(glyphs2.size(), 1);

  auto size = text_blob->getBoundSize();

  EXPECT_GT(size.x, 0.f);
}

int main(int argc, const char** argv) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}