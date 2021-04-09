
#include "src/geometry/math.hpp"

#include <gtest/gtest.h>

#include <array>

TEST(MATH, test_infinit) {
  float nan = glm::asin(2);
  float inf = std::numeric_limits<float>::infinity();
  float big = 3.40282e+038f;

  EXPECT_TRUE(!skity::FloatIsNan(inf));
  EXPECT_TRUE(!skity::FloatIsNan(-inf));
  EXPECT_TRUE(!skity::FloatIsFinite(inf));
  EXPECT_TRUE(!skity::FloatIsFinite(-inf));

  EXPECT_TRUE(skity::FloatIsNan(nan));
  EXPECT_TRUE(!skity::FloatIsNan(big));
  EXPECT_TRUE(!skity::FloatIsNan(-big));
  EXPECT_TRUE(!skity::FloatIsNan(0));

  EXPECT_TRUE(skity::FloatIsFinite(big));
  EXPECT_TRUE(skity::FloatIsFinite(-big));
  EXPECT_TRUE(skity::FloatIsFinite(0));
}

int main(int argc, const char **argv) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}