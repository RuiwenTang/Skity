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
}

int main(int argc, const char** argv) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}