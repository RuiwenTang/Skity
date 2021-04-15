#include "src/geometry/math.hpp"

#include <gtest/gtest.h>

#include <array>
#include <skity/geometry/point.hpp>

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

TEST(MATH, Orientation) {
  skity::Point p1{1, 1, 0, 0};
  skity::Point p2{2, 2, 0, 0};
  skity::Point p3{3, 1, 0, 0};

  skity::Orientation orientation = skity::CalculateOrientation(p1, p2, p3);

  EXPECT_EQ(orientation, skity::Orientation::kClockWise);

  skity::Point p4{-2, -2, 0, 0};
  
  skity::Orientation orientation2 = skity::CalculateOrientation(p1, p4, p3);
  EXPECT_EQ(orientation2, skity::Orientation::kAntiClockWise);
}

int main(int argc, const char **argv) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
