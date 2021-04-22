#include "src/render/path_stroker.hpp"

#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/geometry/quad_construct.hpp"
#include "src/render/cap_proc.hpp"
#include "src/render/join_proc.hpp"

namespace skity {

PathStroker::PathStroker(Path const& src, float radius, float miterLimit,
                         Paint::Cap cap, Paint::Join join, float resScale,
                         bool canIgnoreCenter)
    : radius_(radius), res_scale_(resScale), can_ignore_center_(canIgnoreCenter)
{
  // This is only used when join is miter, but we initialize it here so that it
  // is always defined.
  inv_miter_limit_ = 0.f;

  if (join == Paint::kMiter_Join) {
    if (miterLimit <= Float1) {
      join = Paint::kBevel_Join;
    }
    else {
      inv_miter_limit_ = FloatInvert(miterLimit);
    }
  }

  caper_ = CapProc::MakeCapProc(cap);
  joiner_ = JoinProc::MakeJoinProc(join);

  segment_count_ = -1;
  first_outer_pt_index_in_contour_ = 0;
  prev_is_line_ = false;

  // Need some estimate of how large our final result (fOuter)
  // and our per-contour temp (fInner) will be, so we don't spend
  // extra time repeatedly growing these arrays.
  //
  // 3x for result == inner + outer + join (swag)
  // 1x for inner == 'wag' (worst contour length would be better guess)
  // TODO reserve path points storage

  inv_res_scale_ = FloatInvert(resScale * 4);
  inv_res_scale_squared_ = inv_res_scale_ * inv_res_scale_;
  recursion_depth_ = 0;
}

void PathStroker::moveTo(Point const& pt)
{
  if (segment_count_ > 0) {
    this->finishContour(false, false);
  }

  segment_count_ = 0;
  first_pt_ = prev_pt_ = pt;
  join_complete_ = false;
}

void PathStroker::line_to(Point const& currPt, Vector const& normal)
{
  outer_.lineTo(currPt.x + normal.x, currPt.y + normal.y);
  inner_.lineTo(currPt.x - normal.x, currPt.y - normal.y);
}

static bool has_valid_tangent(const Path::Iter* iter)
{
  Path::Iter copy = *iter;
  Path::Verb verb;
  Point pts[4];

  while (true) {
    verb = copy.next(pts);
    switch (verb) {
      case Path::Verb::kMove:
        return false;
      case Path::Verb::kLine:
        if (pts[0] == pts[1]) {
          continue;
        }
        return true;
      case Path::Verb::kQuad:
      case Path::Verb::kConic:
        if (pts[0] == pts[1] && pts[0] == pts[2] && pts[0] == pts[3]) {
          continue;
        }
        return true;
      case Path::Verb::kClose:
      case Path::Verb::kDone:
        return false;
      default:
        continue;
    }
  }

  return false;
}

PathStroker::ResultType PathStroker::strokeCloseEnough(
    const Point stroke[3], const Point ray[2], QuadConstruct* quadPts) const
{
  std::array<Point, 3> stroke_pts {stroke[0], stroke[1], stroke[2]};
  Point stroke_mid =
      QuadCoeff::EvalQuadAt(stroke_pts, FloatHalf);

  // measure the distance from the curve to the quad-stroke mid-point, compare
  // to radius
  if (PointsWithInDist(ray[0], stroke_mid, inv_res_scale_)) {
    // if the difference is small
    if (SharpAngle(quadPts->quad)) {
      return ResultType::kSplit;
    }
    return ResultType::kQuad;
  }

  // measure the distance to quad's bounds (quick reject) an alternative: look
  // for point in triangle
  if (!ptInQuadBounds(stroke, ray[0])) {
    // if far, subdivide
    return ResultType::kSplit;
  }
  // measure the curve ray distance to the quad-stroke
  float roots[2];
  int32_t root_count = IntersectQuadRay(ray, stroke, roots);
  if (root_count != 1) {
    return ResultType::kSplit;
  }

  Point quad_pt = QuadCoeff::EvalQuadAt(stroke_pts, roots[0]);
  float error = inv_res_scale_ * (Float1 - glm::abs(roots[0] - 0.5f) * 2);
  if (PointsWithInDist(ray[0], quad_pt, error)) {
    // if the difference is small, we're done
    if (SharpAngle(quadPts->quad)) {
      return ResultType::kSplit;
    }
    return ResultType::kQuad;
  }
  // otherwise, subdivide
  return ResultType::kSplit;
}

}  // namespace skity

