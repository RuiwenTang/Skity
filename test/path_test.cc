#include <random>
#include <skity/graphic/path.hpp>

#include "gtest/gtest.h"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/graphic/path_priv.hpp"

TEST(Path, test_iter) {
  skity::Path p;
  skity::Point pts[4];

  skity::Path::Iter no_path_iter;
  EXPECT_EQ(no_path_iter.next(pts), skity::Path::Verb::kDone);

  no_path_iter.setPath(p, false);
  EXPECT_EQ(no_path_iter.next(pts), skity::Path::Verb::kDone);

  no_path_iter.setPath(p, true);
  EXPECT_EQ(no_path_iter.next(pts), skity::Path::Verb::kDone);

  skity::Path::Iter iter{p, false};
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kDone);

  p.reset();
  iter.setPath(p, false);
  EXPECT_FALSE(iter.isClosedContour());

  p.lineTo(1, 1);
  p.close();
  iter.setPath(p, false);
  EXPECT_TRUE(iter.isClosedContour());

  p.reset();
  iter.setPath(p, true);
  EXPECT_FALSE(iter.isClosedContour());
  p.lineTo(1, 1);
  iter.setPath(p, true);
  EXPECT_TRUE(iter.isClosedContour());
  p.moveTo(0, 0);
  p.lineTo(2, 2);
  iter.setPath(p, false);
  EXPECT_FALSE(iter.isClosedContour());

  p.reset();
  p.quadTo(0, 0, 0, 0);
  iter.setPath(p, false);
  iter.next(pts);
  EXPECT_EQ(skity::Path::Verb::kQuad, iter.next(pts));

  p.reset();
  p.conicTo(0, 0, 0, 0, 0.5f);
  iter.setPath(p, false);
  iter.next(pts);
  EXPECT_EQ(skity::Path::Verb::kConic, iter.next(pts));
}

static void check_move(skity::Path::RawIter *iter, float x0, float y0) {
  skity::Point pts[4];
  auto v = iter->next(pts);
  EXPECT_EQ(v, skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, x0);
  EXPECT_FLOAT_EQ(pts[0].y, y0);
}

static void check_line(skity::Path::RawIter *iter, float x1, float y1) {
  skity::Point pts[4];
  auto v = iter->next(pts);
  EXPECT_EQ(v, skity::Path::Verb::kLine);
  EXPECT_FLOAT_EQ(pts[1].x, x1);
  EXPECT_FLOAT_EQ(pts[1].y, y1);
}

static void check_quad(skity::Path::RawIter *iter, float x1, float y1, float x2,
                       float y2) {
  skity::Point pts[4];
  auto v = iter->next(pts);
  EXPECT_EQ(v, skity::Path::Verb::kQuad);
  EXPECT_FLOAT_EQ(pts[1].x, x1);
  EXPECT_FLOAT_EQ(pts[1].y, y1);
  EXPECT_FLOAT_EQ(pts[2].x, x2);
  EXPECT_FLOAT_EQ(pts[2].y, y2);
}

static void check_done(skity::Path *path, skity::Path::RawIter *iter) {
  skity::Point pts[4];
  auto v = iter->next(pts);
  EXPECT_EQ(v, skity::Path::Verb::kDone);
}

static void check_done_and_reset(skity::Path *path,
                                 skity::Path::RawIter *iter) {
  check_done(path, iter);
  path->reset();
}

static void check_path_is_line_and_reset(skity::Path *path, float x1,
                                         float y1) {
  skity::Path::RawIter iter(*path);
  check_move(std::addressof(iter), 0, 0);
  check_line(std::addressof(iter), x1, y1);
  check_done_and_reset(path, std::addressof(iter));
}

static void check_path_is_line(skity::Path *path, float x1, float y1) {
  skity::Path::RawIter iter(*path);
  check_move(std::addressof(iter), 0, 0);
  check_line(std::addressof(iter), x1, y1);
  check_done(path, std::addressof(iter));
}

static void check_path_is_line_pair_and_reset(skity::Path *path, float x1,
                                              float y1, float x2, float y2) {
  skity::Path::RawIter iter(*path);
  check_move(std::addressof(iter), 0, 0);
  check_line(std::addressof(iter), x1, y1);
  check_line(std::addressof(iter), x2, y2);
  check_done_and_reset(path, std::addressof(iter));
}

static void check_path_is_quad_and_reset(skity::Path *path, float x1, float y1,
                                         float x2, float y2) {
  skity::Path::RawIter iter(*path);
  check_move(std::addressof(iter), 0, 0);
  check_quad(std::addressof(iter), x1, y1, x2, y2);
  check_done_and_reset(path, std::addressof(iter));
}

static void check_close(const skity::Path &path) {
  for (int i = 0; i < 2; i++) {
    skity::Path::Iter iter(path, static_cast<bool>(i));
    skity::Point mv;
    skity::Point pts[4];
    skity::Path::Verb v;
    int nMT = 0;
    int nCL = 0;
    skity::PointSet(mv, 0, 0);
    while ((v = iter.next(pts)) != skity::Path::Verb::kDone) {
      switch (v) {
        case skity::Path::Verb::kMove:
          mv = pts[0];
          ++nMT;
          break;
        case skity::Path::Verb::kClose:
          EXPECT_EQ(mv, pts[0]);
          ++nCL;
          break;
        default:
          break;
      }
    }
    EXPECT_TRUE(!i || nMT == nCL);
  }
}

TEST(Path, test_close) {
  skity::Path closePt;
  closePt.moveTo(0, 0);
  closePt.close();
  check_close(closePt);

  skity::Path openPt;
  openPt.moveTo(0, 0);
  check_close(openPt);

  skity::Path empty;
  check_close(empty);
  empty.close();
  check_close(empty);

  skity::Path quad;
  quad.quadTo(Float1, Float1, 10 * Float1, 10 * Float1);
  check_close(quad);
  quad.close();
  check_close(quad);

  skity::Path cubic;
  cubic.cubicTo(Float1, Float1, 10 * Float1, 10 * Float1, 20 * Float1,
                20 * Float1);
  check_close(cubic);
  cubic.close();
  check_close(cubic);

  skity::Path line;
  line.moveTo(Float1, Float1);
  line.lineTo(10 * Float1, 10 * Float1);
  check_close(line);
  line.close();
  check_close(line);

  skity::Path moves;
  moves.moveTo(Float1, Float1);
  moves.moveTo(5 * Float1, 5 * Float1);
  moves.moveTo(Float1, 10 * Float1);
  moves.moveTo(10 * Float1, Float1);
  check_close(moves);
}

TEST(Path, test_arcTo) {
  skity::Path p;

  p.arcTo(0, 0, 1, 2, 1);
  check_path_is_line_and_reset(std::addressof(p), 0, 0);
  p.arcTo(1, 2, 1, 2, 1);
  check_path_is_line_and_reset(std::addressof(p), 1, 2);
  p.arcTo(1, 2, 3, 4, 0);
  check_path_is_line_and_reset(std::addressof(p), 1, 2);
  p.arcTo(1, 2, 0, 0, 1);
  check_path_is_line_and_reset(std::addressof(p), 1, 2);
  p.arcTo(1, 0, 1, 1, 1);
  skity::Point pt;
  EXPECT_TRUE(p.getLastPt(std::addressof(pt)));
  EXPECT_FLOAT_EQ(pt.x, 1.f);
  EXPECT_FLOAT_EQ(pt.y, 1.f);
  p.reset();
  p.arcTo(1, 0, 1, -1, 1);
  EXPECT_TRUE(p.getLastPt(std::addressof(pt)));
  EXPECT_FLOAT_EQ(pt.x, 1.f);
  EXPECT_FLOAT_EQ(pt.y, -1.f);
  {
    p.reset();
    p.moveTo(216, 216);
    // FIXME arcTo is not correct
    // p.arcTo(216, 108, 0, skity::Path::ArcSize::kLarge,
    //         skity::Path::Direction::kCW, 216, 0);
    // p.arcTo(270, 135, 0, skity::Path::ArcSize::kLarge,
    //         skity::Path::Direction::kCCW, 216, 216);
    int n = p.countPoints();
    EXPECT_EQ(p.getPoint(0), p.getPoint(n - 1));
  }
}

TEST(Path, test_quad) {
  skity::Path p;
  p.conicTo(1, 2, 3, 4, -1);
  check_path_is_line_and_reset(std::addressof(p), 3, 4);
  p.conicTo(1.f, 2.f, 3.f, 4.f, FloatInfinity);
  check_path_is_line_pair_and_reset(std::addressof(p), 1, 2, 3, 4);
  p.conicTo(1, 2, 3, 4, 1);
  check_path_is_quad_and_reset(std::addressof(p), 1, 2, 3, 4);
}

TEST(Path_RawIter, test_RawIter) {
  skity::Path p;
  skity::Point pts[4];

  skity::Path::RawIter noPathIter;
  EXPECT_EQ(noPathIter.next(pts), skity::Path::Verb::kDone);
  noPathIter.setPath(p);
  EXPECT_EQ(noPathIter.next(pts), skity::Path::Verb::kDone);

  skity::Path::RawIter iter(p);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kDone);

  p.moveTo(Float1, 0);
  iter.setPath(p);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1);
  EXPECT_FLOAT_EQ(pts[0].y, 0);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kDone);

  // No matter how many moves we add, we should get them all back
  p.moveTo(Float1 * 2, Float1);
  p.moveTo(Float1 * 3, Float1 * 2);
  iter.setPath(p);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1);
  EXPECT_FLOAT_EQ(pts[0].y, 0);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1 * 2);
  EXPECT_FLOAT_EQ(pts[0].y, Float1);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1 * 3);
  EXPECT_FLOAT_EQ(pts[0].y, Float1 * 2);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kDone);

  p.reset();
  p.close();
  iter.setPath(p);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kDone);

  // Move/close sequences
  p.reset();
  p.close();
  p.moveTo(Float1, 0);
  p.close();
  p.close();
  p.moveTo(Float1 * 2, Float1);
  p.close();
  p.moveTo(Float1 * 3, Float1 * 2);
  p.moveTo(Float1 * 4, Float1 * 3);
  p.close();
  iter.setPath(p);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1);
  EXPECT_FLOAT_EQ(pts[0].y, 0);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kClose);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1 * 2);
  EXPECT_FLOAT_EQ(pts[0].y, Float1);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kClose);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1 * 3);
  EXPECT_FLOAT_EQ(pts[0].y, Float1 * 2);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_FLOAT_EQ(pts[0].x, Float1 * 4);
  EXPECT_FLOAT_EQ(pts[0].y, Float1 * 3);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kClose);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kDone);

  // Generate random paths and verify
  skity::Point randomPts[25];
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      skity::PointSet(randomPts[i * 5 + j], Float1 * float(i),
                      Float1 *float(j));
    }
  }

  std::vector<skity::Path::Verb> verbs = {
      skity::Path::Verb::kMove,  skity::Path::Verb::kLine,
      skity::Path::Verb::kQuad,  skity::Path::Verb::kConic,
      skity::Path::Verb::kCubic, skity::Path::Verb::kClose};
  std::vector<skity::Point> expectedPts(31);
  std::vector<skity::Path::Verb> expectedVerbs(22);
  skity::Path::Verb nextVerb;

  for (int i = 0; i < 2; i++) {
    p.reset();
    bool lastWasClose = true;
    bool haveMoteTo = false;
    skity::Point lastMoveToPt = {0, 0, 0, 1};
    int numPoints = 0;
    std::random_device
        rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with
                             // rd()
    int numVerbs = std::uniform_int_distribution<int>{0, 9}(gen);
    int numIterVerbs = 0;
    for (int j = 0; j < numVerbs; j++) {
      do {
        nextVerb =
            verbs[std::uniform_int_distribution<int>(0, verbs.size() - 1)(gen)];
      } while (lastWasClose && nextVerb == skity::Path::Verb::kClose);
      switch (nextVerb) {
        case skity::Path::Verb::kMove:
          expectedPts[numPoints] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          p.moveTo(expectedPts[numPoints].x, expectedPts[numPoints].y);
          numPoints += 1;
          lastWasClose = false;
          haveMoteTo = true;
          break;
        case skity::Path::Verb::kLine:
          if (!haveMoteTo) {
            expectedPts[numPoints++] = lastMoveToPt;
            expectedVerbs[numIterVerbs++] = skity::Path::Verb::kMove;
            haveMoteTo = true;
          }
          expectedPts[numPoints] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          p.lineTo(expectedPts[numPoints].x, expectedPts[numPoints].y);
          numPoints += 1;
          lastWasClose = false;
          break;
        case skity::Path::Verb::kQuad:
          if (!haveMoteTo) {
            expectedPts[numPoints++] = lastMoveToPt;
            expectedVerbs[numIterVerbs++] = skity::Path::Verb::kMove;
            haveMoteTo = true;
          }
          expectedPts[numPoints] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          expectedPts[numPoints + 1] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          p.quadTo(expectedPts[numPoints].x, expectedPts[numPoints].y,
                   expectedPts[numPoints + 1].x, expectedPts[numPoints + 1].y);
          numPoints += 2;
          lastWasClose = false;
          break;
        case skity::Path::Verb::kConic:
          if (!haveMoteTo) {
            expectedPts[numPoints++] = lastMoveToPt;
            expectedVerbs[numIterVerbs++] = skity::Path::Verb::kMove;
            haveMoteTo = true;
          }
          expectedPts[numPoints] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          expectedPts[numPoints + 1] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          p.conicTo(
              expectedPts[numPoints].x, expectedPts[numPoints].y,
              expectedPts[numPoints + 1].x, expectedPts[numPoints + 1].y,
              std::uniform_real_distribution<float>(0.f, 0.999f)(gen) * 4);
          numPoints += 2;
          lastWasClose = false;
          break;
        case skity::Path::Verb::kCubic:
          if (!haveMoteTo) {
            expectedPts[numPoints++] = lastMoveToPt;
            expectedVerbs[numIterVerbs++] = skity::Path::Verb::kMove;
            haveMoteTo = true;
          }
          expectedPts[numPoints] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          expectedPts[numPoints + 1] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          expectedPts[numPoints + 2] =
              randomPts[std::uniform_int_distribution<int>(0, 24)(gen)];
          p.cubicTo(expectedPts[numPoints].x, expectedPts[numPoints].y,
                    expectedPts[numPoints + 1].x, expectedPts[numPoints + 1].y,
                    expectedPts[numPoints + 2].x, expectedPts[numPoints + 2].y);
          numPoints += 3;
          lastWasClose = false;
          break;
        case skity::Path::Verb::kClose:
          p.close();
          haveMoteTo = false;
          lastWasClose = true;
          break;
        default:
          break;
      }
      expectedVerbs[numIterVerbs++] = nextVerb;
    }

    iter.setPath(p);
    numVerbs = numIterVerbs;
    numIterVerbs = 0;
    int numIterPts = 0;

    skity::Point lastMoveTo;
    skity::Point lastPt;
    skity::PointSet(lastMoveTo, 0, 0);
    skity::PointSet(lastPt, 0, 0);
    while ((nextVerb = iter.next(pts)) != skity::Path::Verb::kDone) {
      EXPECT_EQ(nextVerb, expectedVerbs[numIterVerbs]);
      numIterVerbs++;
      switch (nextVerb) {
        case skity::Path::Verb::kMove:
          EXPECT_TRUE(numIterPts < numPoints);
          EXPECT_FLOAT_EQ(pts[0].x, expectedPts[numIterPts].x);
          EXPECT_FLOAT_EQ(pts[0].y, expectedPts[numIterPts].y);
          if (pts[0] != expectedPts[numIterPts]) {
            int ss = 123;
          }
          lastPt = lastMoveTo = pts[0];
          numIterPts += 1;
          break;
        case skity::Path::Verb::kLine:
          EXPECT_LT(numIterPts, numPoints + 1);
          EXPECT_EQ(pts[0], lastPt);
          EXPECT_EQ(pts[1], expectedPts[numIterPts]);
          lastPt = pts[1];
          numIterPts += 1;
          break;
        case skity::Path::Verb::kQuad:
        case skity::Path::Verb::kConic:
          EXPECT_LT(numIterPts, numPoints + 2);
          EXPECT_EQ(pts[0], lastPt);
          EXPECT_EQ(pts[1], expectedPts[numIterPts]);
          EXPECT_EQ(pts[2], expectedPts[numIterPts + 1]);
          lastPt = pts[2];
          numIterPts += 2;
          break;
        case skity::Path::Verb::kCubic:
          EXPECT_LT(numIterPts, numPoints + 3);
          EXPECT_EQ(pts[0], lastPt);
          EXPECT_EQ(pts[1], expectedPts[numIterPts]);
          EXPECT_EQ(pts[2], expectedPts[numIterPts + 1]);
          EXPECT_EQ(pts[3], expectedPts[numIterPts + 2]);
          lastPt = pts[3];
          numIterPts += 3;
          break;
        case skity::Path::Verb::kClose:
          lastPt = lastMoveTo;
          break;
        default:
          break;
      }
    }
    EXPECT_EQ(numIterPts, numPoints);
    EXPECT_EQ(numIterVerbs, numVerbs);
  }
}

TEST(Path, bad_case) {
  skity::Path path;
  skity::Point randomPts[25];
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      skity::PointSet(randomPts[i * 5 + j], Float1 * float(i),
                      Float1 *float(j));
    }
  }

  path.moveTo(randomPts[0].x, randomPts[0].y);
  path.cubicTo(randomPts[1].x, randomPts[1].y, randomPts[2].x, randomPts[2].y,
               randomPts[3].x, randomPts[3].y);
  path.cubicTo(randomPts[3].x, randomPts[3].y, randomPts[5].x, randomPts[5].y,
               randomPts[6].x, randomPts[6].y);
  path.close();
  path.moveTo(randomPts[7].x, randomPts[7].y);
  skity::Path::RawIter iter(path);

  skity::Point pts[4];
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kCubic);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kCubic);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kClose);
  EXPECT_EQ(iter.next(pts), skity::Path::Verb::kMove);
  EXPECT_EQ(pts[0], randomPts[7]);
}

TEST(path, test_range_iter) {
  skity::Path path;
  skity::PathPriv::Iterate iterate{path};

  EXPECT_TRUE(iterate.begin() == iterate.end());

  path.moveTo(Float1, 0);
  iterate = skity::PathPriv::Iterate(path);

  auto iter = iterate.begin();
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1);
    EXPECT_EQ(pts[0].y, 0.f);
  }
  EXPECT_TRUE(iter == iterate.end());

  path.moveTo(Float1 * 2, Float1);
  path.moveTo(Float1 * 3, Float1 * 2);
  iterate = skity::PathPriv::Iterate(path);
  iter = iterate.begin();
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1);
    EXPECT_EQ(pts[0].y, 0.f);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1 * 2);
    EXPECT_EQ(pts[0].y, Float1);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1 * 3);
    EXPECT_EQ(pts[0].y, Float1 * 2);
  }

  EXPECT_TRUE(iter == iterate.end());

  path.reset();
  path.close();
  iterate = skity::PathPriv::Iterate(path);
  EXPECT_TRUE(iterate.begin() == iterate.end());

  path.reset();
  path.close();  // Not stored, no purpose
  path.moveTo(Float1, 0);
  path.close();
  path.close();  // Not stored, no purpose
  path.moveTo(Float1 * 2, Float1);
  path.close();
  path.moveTo(Float1 * 3, Float1 * 2);
  path.moveTo(Float1 * 4, Float1 * 3);
  path.close();

  iterate = skity::PathPriv::Iterate(path);
  iter = iterate.begin();
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1);
    EXPECT_EQ(pts[0].y, 0);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kClose);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1 * 2);
    EXPECT_EQ(pts[0].y, Float1);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kClose);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1 * 3);
    EXPECT_EQ(pts[0].y, Float1 * 2);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kMove);
    EXPECT_EQ(pts[0].x, Float1 * 4);
    EXPECT_EQ(pts[0].y, Float1 * 3);
  }
  {
    auto ret = *iter++;
    auto verb = std::get<0>(ret);
    EXPECT_EQ(verb, skity::Path::Verb::kClose);
  }
  EXPECT_TRUE(iter == iterate.end());
}

int main(int argc, const char **argv) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
