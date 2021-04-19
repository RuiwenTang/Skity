#include "src/render/stroke.hpp"

#include <skity/geometry/point.hpp>

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

  float dot = glm::dot(smaller, larger);
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

Stroke::Stroke() {}

}  // namespace skity

