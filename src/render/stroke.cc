#include "src/render/stroke.hpp"

#include <skity/geometry/point.hpp>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

enum {
  kTangent_RecursiveLimit,
  kCubic_RecursiveLimit,
  kConic_RecursiveLimit,
  kQuad_RecursiveLimit,
};

static bool points_within_dist(Point const& nearPt, Point const& farPt,
                               float limit)
{
  return PointDistanceToSqd(nearPt, farPt) <= limit * limit;
}

static bool sharp_angle(const Point quad[3])
{
  Vector smaller = quad[1] - quad[0];
  Vector larger = quad[1] - quad[2];

  float smallLen = PointLengthSqd(smaller);
  float largerLen = PointLengthSqd(larger);

  if (smallLen > largerLen) {
    std::swap(smaller, larger);
    largerLen = smallLen;
  }

  if (!PointSetLength<false>(smaller, smaller.x, smaller.y, largerLen)) {
    return false;
  }

  float dot = glm::dot(glm::vec2{smaller}, glm::vec2{larger});
  return dot > 0;
}

static int intersect_quad_ray(const Point line[2], const Point quad[3],
                              float roots[2])
{
  Vector vec = line[1] - line[0];
  float r[3];
  for (int n = 0; n < 3; n++) {
    r[n] = (quad[n].y - line[0].y) * vec.x - (quad[n].x - line[0].x) * vec.y;
  }

  float A = r[2];
  float B = r[1];
  float C = r[0];

  A += C - 2 * B;  // A = a - 2 * b + c
  B -= C;          // B = -(b - c)

  return FindUnitQuadRoots(A, 2 * B, C, roots);
}

static bool degenerate_vector(Vector const& v)
{
  return !PointCanNormalize(v.x, v.y);
}

/**
 * Returns the distance squared from the point to the line segment
 *
 * @param pt          point
 * @param lineStart   start point of the line
 * @param lineEnd     end point of the line
 *
 * @return distance squared value
 */
static float pt_to_line(Point const& pt, Point const& lineStart,
                        Point const& lineEnd)
{
  Vector dxy = lineEnd - lineStart;
  Vector ab0 = pt - lineStart;

  float number = VectorDotProduct(dxy, ab0);
  float denom = VectorDotProduct(dxy, dxy);
  float t = SkityIEEEFloatDivided(number, denom);
  if (t >= 0 && t <= 1) {
    Point hit;
    hit.x = lineStart.x * (1 - t) + lineEnd.x * t;
    hit.y = lineStart.y * (1 - t) + lineEnd.y * t;
    hit.z = 0;
    hit.w = 1;
    return PointDistanceToSqd(hit, pt);
  }
  else {
    return PointDistanceToSqd(pt, lineStart);
  }
}

/**
 * Given a cubic, determine if all four points are in a line
 *
 * @param cubic[4] cubic points
 *
 * @return true if inner points is close to a line.
 */
static bool cubic_in_line(const Point cubic[4])
{
  float pt_max = -1;
  int32_t outer1 = 0;
  int32_t outer2 = 0;
  for (int index = 0; index < 3; index++) {
    for (int inner = index + 1; inner < 4; inner++) {
      Vector test_diff = cubic[inner] - cubic[index];
      float test_max = std::max(std::abs(test_diff.x), std::abs(test_diff.y));
      if (pt_max < test_max) {
        outer1 = index;
        outer2 = inner;
        pt_max = test_max;
      }
    }
  }

  int32_t mid1 = (1 + (2 >> outer2)) >> outer1;
  int32_t mid2 = outer1 ^ outer2 ^ mid1;
  float line_slop = pt_max * pt_max * 0.00001f;
  return pt_to_line(cubic[mid1], cubic[outer1], cubic[outer2]) <= line_slop &&
         pt_to_line(cubic[mid2], cubic[outer1], cubic[outer2]) <= line_slop;
}

/**
 * Given quad, see if all three points are in a line
 *
 * @param quad[3]
 *
 * @return true if all three points are in a line
 */
static bool quad_in_line(const Point quad[3])
{
  float pt_max = -1;
  int32_t outer1 = 0;
  int32_t outer2 = 0;
  for (int index = 0; index < 2; index++) {
    for (int inner = index + 1; inner < 3; inner++) {
      Vector test_diff = quad[inner] - quad[index];
      float test_max = std::max(std::abs(test_diff.x), std::abs(test_diff.y));
      if (pt_max < test_max) {
        outer1 = index;
        outer2 = inner;
        pt_max = test_max;
      }
    }
  }

  int mid = outer1 ^ outer2 ^ 3;
  const float kCurvatureSlop = 0.000005f;
  float lineSlop = pt_max * pt_max * kCurvatureSlop;
  return pt_to_line(quad[mid], quad[outer1], quad[outer2]) <= lineSlop;
}

static bool conic_in_line(Conic const& conic)
{
  return quad_in_line(conic.pts);
}

/// If src == dst, then we use a temp path to record the stroke, and then swap
/// its contents with src when we're done.
class AutoTmpPath {
 public:
  AutoTmpPath(Path const& src, Path** dst) : src_(src)
  {
    if (&src == *dst) {
      *dst = std::addressof(tmp_dst_);
      swap_with_src_ = true;
    }
    else {
      (*dst)->reset();
      swap_with_src_ = false;
    }
  }

  ~AutoTmpPath()
  {
    if (swap_with_src_) {
      tmp_dst_.swap(*const_cast<Path*>(&src_));
    }
  }

 private:
  Path tmp_dst_;
  Path const& src_;
  bool swap_with_src_;
};

Stroke::Stroke() {}

}  // namespace skity

