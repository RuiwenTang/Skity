#include "src/graphic/path_measure.hpp"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <skity/graphic/path.hpp>

#include "gtest/gtest.h"
#include "src/geometry/math.hpp"

static void test_small_segment1() {
  skity::Path path;
  std::array<skity::Point, 3> pts = {
      skity::Point{100000, 100000, 0.f, 1.f},
      // big jump between these points, makes a big segment
      skity::Point{1.0005f, 0.9999f, 0.f, 1.f},
      // tiny (non-zero) jump between these points
      skity::Point{Float1, Float1, 0.f, 1.f},
  };

  path.moveTo(pts[0]);
  for (size_t i = 1; i < pts.size(); i++) {
    path.lineTo(pts[i]);
  }
  skity::PathMeasure meas{path, false};
  /*  this would assert (before a fix) because we added a segment with
  the same length as the prev segment, due to the follow (bad) pattern

  d = distance(pts[0], pts[1]);
  distance += d;
  seg->fDistance = distance;

  SkASSERT(d > 0);    // TRUE
  SkASSERT(seg->fDistance > prevSeg->fDistance);  // FALSE

  This 2nd assert failes because (distance += d) didn't affect distance
  because distance >>> d.
*/
  meas.getLength();
}

static void test_small_segment2() {
  skity::Path path;
  std::array<skity::Point, 5> pts = {
      skity::Point{0, 0, 0, 1},
      skity::Point{100000000000.0f, 100000000000.0f, 0, 1},
      skity::Point{0, 0, 0, 1},
      skity::Point{10, 10, 0, 1},
      skity::Point{0, 0, 0, 1},
  };

  path.moveTo(pts[0]);
  for (size_t i = 1; i < pts.size(); i += 2) {
    path.quadTo(pts[i], pts[i + 1]);
  }

  skity::PathMeasure meas{path, false};
  meas.getLength();
}

static void test_small_segment3() {
  skity::Path path;
  std::array<skity::Point, 6> pts = {
      skity::Point{0, 0, 0, 1},
      skity::Point{100000000000.0f, 100000000000.0f, 0, 1},
      skity::Point{0, 0, 0, 1},
      skity::Point{10, 10, 0, 1},
      skity::Point{0, 0, 0, 1},
      skity::Point{10, 10, 0, 1},
  };

  path.moveTo(pts[0]);
  for (size_t i = 1; i < pts.size(); i += 3) {
    path.cubicTo(pts[i], pts[i + 1], pts[i + 2]);
  }

  skity::PathMeasure meas{path, false};
  meas.getLength();
}

TEST(PathMeasure, getLength) {
  skity::Path path;

  path.moveTo(0, 0);
  path.lineTo(Float1, 0);
  path.lineTo(Float1, Float1);
  path.lineTo(0, Float1);

  skity::PathMeasure meas{path, true};
  float length = meas.getLength();
  EXPECT_EQ(length, 4 * Float1);

  path.reset();
  path.moveTo(0, 0);
  path.lineTo(Float1 * 3, Float1 * 4);
  meas.setPath(&path, false);
  length = meas.getLength();
  EXPECT_EQ(length, Float1 * 5);

  path.reset();
  path.addCircle(0, 0, Float1);
  meas.setPath(&path, true);
  length = meas.getLength();
  // EXPECT_FLOAT_EQ(length, 2 * glm::pi<float>());

  // Test the behavior following a close not followed by a move.
  path.reset();
  path.lineTo(Float1, 0);
  path.lineTo(Float1, Float1);
  path.lineTo(0, Float1);
  path.close();
  path.lineTo(-Float1, 0);
  meas.setPath(&path, false);

  length = meas.getLength();
  EXPECT_FLOAT_EQ(length, Float1 * 4);
  meas.nextContour();
  length = meas.getLength();
  EXPECT_FLOAT_EQ(Float1, length);

  skity::Point position;
  skity::Vector tangent;
  EXPECT_TRUE(meas.getPosTan(FloatHalf, &position, &tangent));
  EXPECT_FLOAT_EQ(position.x, -FloatHalf);
  EXPECT_FLOAT_EQ(position.y, 0.f);
  EXPECT_FLOAT_EQ(tangent.x, -Float1);
  EXPECT_FLOAT_EQ(tangent.y, 0);

  // Test degenerate paths
  path.reset();
  path.moveTo(0, 0);
  path.lineTo(0, 0);
  path.lineTo(Float1, 0);
  path.quadTo(Float1, 0, Float1, 0);
  path.quadTo(Float1, Float1, Float1, Float1 * 2);
  path.cubicTo(Float1, Float1 * 2, Float1, Float1 * 2, Float1, Float1 * 2);
  path.cubicTo(Float1 * 2, Float1 * 2, Float1 * 3, Float1 * 2, Float1 * 4,
               Float1 * 2);

  meas.setPath(&path, false);
  length = meas.getLength();
  EXPECT_FLOAT_EQ(length, Float1 * 6);

  EXPECT_TRUE(meas.getPosTan(FloatHalf, &position, &tangent));
  EXPECT_FLOAT_EQ(position.x, FloatHalf);
  EXPECT_FLOAT_EQ(position.y, 0.f);
  EXPECT_FLOAT_EQ(tangent.x, Float1);
  EXPECT_FLOAT_EQ(tangent.y, 0.f);

  EXPECT_TRUE(meas.getPosTan(2.5f, &position, &tangent));
  EXPECT_FLOAT_EQ(position.x, Float1);
  EXPECT_FLOAT_EQ(position.y, 1.5f);
  EXPECT_FLOAT_EQ(tangent.x, 0);
  EXPECT_FLOAT_EQ(tangent.y, Float1);

  EXPECT_TRUE(meas.getPosTan(4.5f, &position, &tangent));
  EXPECT_FLOAT_EQ(position.x, 2.5f);
  EXPECT_FLOAT_EQ(position.y, 2.f);
  EXPECT_FLOAT_EQ(tangent.x, Float1);
  EXPECT_FLOAT_EQ(tangent.y, 0.f);

  path.reset();
  path.moveTo(0, 0);
  path.lineTo(Float1, 0);
  path.moveTo(Float1, Float1);
  path.moveTo(Float1 * 2, Float1 * 2);
  path.lineTo(Float1, Float1 * 2);
  meas.setPath(&path, false);

  length = meas.getLength();
  EXPECT_FLOAT_EQ(length, Float1);
  EXPECT_TRUE(meas.getPosTan(FloatHalf, &position, &tangent));
  EXPECT_FLOAT_EQ(position.x, FloatHalf);
  EXPECT_FLOAT_EQ(position.y, 0.f);
  EXPECT_FLOAT_EQ(tangent.x, Float1);
  EXPECT_FLOAT_EQ(tangent.y, 0.f);
  meas.nextContour();
  length = meas.getLength();
  EXPECT_FLOAT_EQ(length, Float1);
  EXPECT_TRUE(meas.getPosTan(FloatHalf, &position, &tangent));
  EXPECT_FLOAT_EQ(position.x, 1.5f);
  EXPECT_FLOAT_EQ(position.y, 2.f);
  EXPECT_FLOAT_EQ(tangent.x, -Float1);
  EXPECT_FLOAT_EQ(tangent.y, 0.f);

  test_small_segment1();
  test_small_segment2();
}

int main(int argc, const char** argv) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}