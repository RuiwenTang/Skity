#include "src/render/path_stroker.hpp"

#include <cassert>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/geometry/quad_construct.hpp"
#include "src/render/cap_proc.hpp"
#include "src/render/join_proc.hpp"

namespace skity {

enum {
  kTangent_RecursiveLimit,
  kCubic_RecursiveLimit,
  kConic_RecursiveLimit,
  kQuad_RecursiveLimit,
};

static const int kRecursiveLimit[] = {
    5 * 3,
    26 * 3,
    11 * 3,
    11 * 3,
};

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

PathStroker::~PathStroker() = default;

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
  std::array<Point, 3> stroke_pts{stroke[0], stroke[1], stroke[2]};
  Point stroke_mid = QuadCoeff::EvalQuadAt(stroke_pts, FloatHalf);

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

PathStroker::ResultType PathStroker::tangentsMeet(const Point* cubic,
                                                  QuadConstruct* quadPts)
{
  this->cubicQuadEnds(cubic, quadPts);
  return this->intersectRay(quadPts, IntersectRayType::kResult);
}

static bool set_normal_unitnormal(Point const& before, Point const& after,
                                  float scale, float radius, Vector* normal,
                                  Vector* unitNormal)
{
  if (!VectorSetNormal(*unitNormal, (after.x - before.x) * scale,
                       (after.y - before.y) * scale)) {
    return false;
  }

  PointRotateCCW(unitNormal);
  PointScale(*unitNormal, radius, normal);
  return true;
}

static bool set_normal_unitnormal(Vector const& vec, float radius,
                                  Vector* normal, Vector* unitNormal)
{
  if (!VectorSetNormal(*unitNormal, vec.x, vec.y)) {
    return false;
  }

  PointRotateCCW(unitNormal);
  PointScale(*unitNormal, radius, normal);
  return true;
}

void PathStroker::lineTo(Point const& currPt, const Path::Iter* iter)
{
  bool teenyLine =
      PointEqualsWithinTolerance(prev_pt_, currPt, NearlyZero * inv_res_scale_);
  if (Paint::Cap::kButt_Cap == caper_->type() && teenyLine) {
    return;
  }

  if (teenyLine && (join_complete_ || (iter && has_valid_tangent(iter)))) {
    return;
  }

  Vector normal, unitNormal;

  if (!this->preJoinTo(currPt, std::addressof(normal),
                       std::addressof(unitNormal), true)) {
    return;
  }

  this->line_to(currPt, normal);
  this->postJoinTo(currPt, normal, unitNormal);
}

void PathStroker::quadTo(Point const& pt1, Point const& pt2)
{
  std::array<Point, 3> quad{prev_pt_, pt1, pt2};
  Point reduction;
  ReductionType reduction_type =
      CheckQuadLinear(quad.data(), std::addressof(reduction));
  if (reduction_type == ReductionType::kPoint_ReductionType) {
    // If the stroke consists of a moveTo followed by a degenerate curve, treat
    // it as if it were followed by zero-length line. Lines without length can
    // have square and round end caps
    this->lineTo(pt2);
    return;
  }

  if (reduction_type == ReductionType::kLine_ReductionType) {
    this->lineTo(pt2);
    return;
  }
  if (reduction_type == ReductionType::kDegenerate_ReductionType) {
    this->lineTo(reduction);
    auto save_joiner = std::move(joiner_);
    joiner_ = JoinProc::MakeJoinProc(Paint::kRound_Join);
    this->lineTo(pt2);
    joiner_ = std::move(save_joiner);
    return;
  }

  assert(reduction_type == ReductionType::kQuad_ReductionType);

  Vector normalAB, unitAB, normalBC, unitBC;

  if (!this->preJoinTo(pt1, std::addressof(normalAB), std::addressof(unitAB),
                       false)) {
    this->lineTo(pt2);
    return;
  }

  QuadConstruct quad_pts{};
  this->init(StrokeType::kOuter_StrokeType, std::addressof(quad_pts), 0, 1);
  this->quadStroke(quad.data(), std::addressof(quad_pts));
  this->init(StrokeType::kInner_StrokeType, std::addressof(quad_pts), 0, 1);
  this->quadStroke(quad.data(), std::addressof(quad_pts));

  this->setQuadEndNormal(quad.data(), normalAB, unitAB,
                         std::addressof(normalBC), std::addressof(unitBC));

  this->postJoinTo(pt2, normalBC, unitBC);
}

void PathStroker::conicTo(Point const& pt1, Point const& pt2, float weight)
{
  Conic conic{prev_pt_, pt1, pt2, weight};
  Point reduction;
  ReductionType reduction_type =
      CheckConicLinear(conic, std::addressof(reduction));

  if (reduction_type == ReductionType::kPoint_ReductionType) {
    this->lineTo(pt2);
    return;
  }

  if (reduction_type == ReductionType::kLine_ReductionType) {
    this->lineTo(pt2);
    return;
  }

  if (reduction_type == ReductionType::kDegenerate_ReductionType) {
    this->lineTo(reduction);
    auto save_joiner = std::move(joiner_);
    joiner_ = JoinProc::MakeJoinProc(Paint::kRound_Join);
    this->lineTo(pt2);
    joiner_ = std::move(save_joiner);
    return;
  }

  Vector normalAB, unitAB, normalBC, unitBC;
  if (!this->preJoinTo(pt1, std::addressof(normalAB), std::addressof(unitAB),
                       false)) {
    this->lineTo(pt2);
    return;
  }

  QuadConstruct quad_pts;
  this->init(StrokeType::kOuter_StrokeType, std::addressof(quad_pts), 0, 1);
  this->conicStroke(conic, std::addressof(quad_pts));
  this->init(StrokeType::kInner_StrokeType, std::addressof(quad_pts), 0, 1);
  this->conicStroke(conic, std::addressof(quad_pts));
  this->setConicEndNormal(conic, normalAB, unitAB, std::addressof(normalBC),
                          std::addressof(unitBC));

  this->postJoinTo(pt2, normalBC, unitBC);
}

void PathStroker::cubicTo(Point const& pt1, Point const& pt2, Point const& pt3)
{
  std::array<Point, 4> cubic{prev_pt_, pt1, pt2, pt3};
  std::array<Point, 3> reductions;
  const Point* tangent_pt;
  ReductionType reduction_type = CheckCubicLinear(
      cubic.data(), reductions.data(), std::addressof(tangent_pt));
  if (reduction_type == ReductionType::kPoint_ReductionType) {
    this->lineTo(pt3);
    return;
  }
  if (reduction_type == ReductionType::kLine_ReductionType) {
    this->lineTo(pt3);
    return;
  }

  if (kDegenerate_ReductionType <= reduction_type &&
      kDegenerate3_ReductionType >= reduction_type) {
    this->lineTo(reductions[0]);
    auto save_joiner = std::move(joiner_);
    joiner_ = JoinProc::MakeJoinProc(Paint::kRound_Join);
    if (kDegenerate2_ReductionType <= reduction_type) {
      this->lineTo(reductions[1]);
    }
    if (reduction_type == ReductionType::kDegenerate3_ReductionType) {
      this->lineTo(reductions[2]);
    }
    this->lineTo(reductions[3]);
    joiner_ = std::move(save_joiner);
    return;
  }

  Vector normalAB, unitAB, normalCD, unitCD;
  if (!this->preJoinTo(*tangent_pt, std::addressof(normalAB),
                       std::addressof(unitAB), false)) {
    this->lineTo(pt3);
    return;
  }
  std::array<float, 2> t_values;
  int count = FindCubicInflections(cubic.data(), t_values.data());
  float last_t = 0.f;
  for (int32_t index = 0; index <= count; index++) {
    float next_t = index < count ? t_values[index] : 1;
    QuadConstruct quad_pts;
    this->init(StrokeType::kOuter_StrokeType, std::addressof(quad_pts), last_t, next_t);
    this->cubicStroke(cubic.data(), std::addressof(quad_pts));
    this->init(StrokeType::kInner_StrokeType, std::addressof(quad_pts), last_t, next_t);
    this->cubicStroke(cubic.data(), std::addressof(quad_pts));
    last_t = next_t;
  }
  float cusp = FindCubicCusp(cubic.data());
  if (cusp > 0) {
    Point cusp_loc;
    CubicCoeff::EvalCubicAt(cubic.data(), cusp, std::addressof(cusp_loc),
                            nullptr, nullptr);
    cusper_.addCircle(cusp_loc.x, cusp_loc.y, radius_);
  }
  // emit the join even if one stroke succeeded but the last one failed this
  // avoids reversiong an inner stroke with a partial path followed by another
  // moveTo
  this->setCubicEndNormal(cubic.data(), normalAB, unitAB,
                          std::addressof(normalCD), std::addressof(unitCD));
  this->postJoinTo(pt3, normalCD, unitCD);
}

void PathStroker::close(bool is_line) { this->finishContour(true, is_line); }

void PathStroker::done(Path* dst, bool is_line)
{
  this->finishContour(false, is_line);
  dst->swap(outer_);
}

void PathStroker::setQuadEndNormal(const Point* quad, Vector const& normalAB,
                                   Vector const& unitNormalAB, Vector* normalBC,
                                   Vector* unitNormalBC)
{
  if (!set_normal_unitnormal(quad[1], quad[2], res_scale_, radius_, normalBC,
                             unitNormalBC)) {
    *normalBC = normalAB;
    *unitNormalBC = unitNormalAB;
  }
}

void PathStroker::init(PathStroker::StrokeType stroke_type,
                       QuadConstruct* quad_pts, float t_start, float t_end)
{
  stroke_type_ = stroke_type;
  found_tangents_ = false;
  quad_pts->init(t_start, t_end);
}

bool PathStroker::ptInQuadBounds(const Point* quad, Point const& pt) const
{
  float x_min = std::min(std::min(quad[0].x, quad[1].x), quad[2].x);
  if (pt.x + inv_res_scale_ < x_min) {
    return false;
  }
  float x_max = std::max(std::max(quad[0].x, quad[1].x), quad[2].x);
  if (pt.x - inv_res_scale_ > x_max) {
    return false;
  }

  float y_min = std::min(std::min(quad[0].y, quad[1].y), quad[2].y);
  if (pt.y + inv_res_scale_ < y_min) {
    return false;
  }
  float y_max = std::max(std::max(quad[0].y, quad[1].y), quad[2].y);
  if (pt.y - inv_res_scale_ > y_max) {
    return false;
  }

  return true;
}

void PathStroker::setRayPts(Point const& pt, Vector* dxy, Point* on_pt,
                            Point* tangent) const
{
  if (!PointSetLength<false>(*dxy, dxy->x, dxy->y, radius_)) {
    VectorSet(*dxy, radius_, 0);
  }

  dxy->z = 0;
  dxy->w = 0;
  auto axis_flip = static_cast<float>(stroke_type_);
  on_pt->x = pt.x + axis_flip * dxy->y;
  on_pt->y = pt.y - axis_flip * dxy->x;
  if (tangent) {
    tangent->x = on_pt->x + dxy->x;
    tangent->y = on_pt->y + dxy->y;
  }
}

bool PathStroker::preJoinTo(Point const& curr_ptr, Vector* normal,
                            Vector* unitNormal, bool is_line)
{
  assert(segment_count_ >= 0);

  float prev_x = prev_pt_.x;
  float prev_y = prev_pt_.y;

  if (!set_normal_unitnormal(prev_pt_, curr_ptr, res_scale_, radius_, normal,
                             unitNormal)) {
    if (caper_->type() == Paint::kButt_Cap) {
      return false;
    }
    // Square caps and round caps draw even if the segment length is zero.
    // Since the zero length segment has no direction, set the orientation to
    // upright as default orientation
    VectorSet(*normal, radius_, 0);
    VectorSet(*unitNormal, 1, 0);
  }

  if (segment_count_ == 0) {
    first_normal_ = *normal;
    first_unit_normal_ = *unitNormal;
    PointSet(first_outer_pt_, prev_x + normal->x, prev_y + normal->y);

    outer_.moveTo(first_outer_pt_.x, first_outer_pt_.y);
    inner_.moveTo(prev_x - normal->x, prev_y - normal->y);
  }
  else {
    // there is a previous segment
    joiner_->proc(std::addressof(outer_), std::addressof(inner_),
                  prev_unit_normal_, prev_pt_, *unitNormal, radius_,
                  inv_miter_limit_, prev_is_line_, is_line);
  }
  prev_is_line_ = is_line;
  return true;
}

void PathStroker::postJoinTo(Point const& curr_ptr, Vector const& normal,
                             Vector const& unitNormal)
{
  join_complete_ = true;
  prev_pt_ = curr_ptr;
  prev_unit_normal_ = unitNormal;
  prev_normal_ = normal;
  segment_count_ += 1;
}

void PathStroker::finishContour(bool close, bool is_line)
{
  if (segment_count_ > 0) {
    Point pt;
    if (close) {
      joiner_->proc(std::addressof(outer_), std::addressof(inner_),
                    prev_unit_normal_, prev_pt_, first_unit_normal_, radius_,
                    inv_miter_limit_, prev_is_line_, is_line);
      outer_.close();

      if (can_ignore_center_) {
        // If we can ignore the center just make sure the larger of the two
        // paths is preserved and don't add the smaller one.
        // TODO implement path.getBounds
      }
      else {
        // now add inner as its own contour
        inner_.getLastPt(std::addressof(pt));
        outer_.moveTo(pt);
        outer_.reversePathTo(inner_);
        outer_.close();
      }
    }
    else {
      // add caps to start and end

      // cap the end
      inner_.getLastPt(std::addressof(pt));
      caper_->proc(std::addressof(outer_), prev_pt_, prev_normal_, pt,
                   is_line ? std::addressof(inner_) : nullptr);
      outer_.reversePathTo(inner_);
      // cap the start
      caper_->proc(std::addressof(outer_), first_pt_, -first_normal_,
                   first_outer_pt_,
                   prev_is_line_ ? std::addressof(inner_) : nullptr);
      outer_.close();
    }
    if (!cusper_.isEmpty()) {
      outer_.addPath(cusper_);
      cusper_.reset();
    }
  }
  inner_.reset();
  segment_count_ = -1;
  first_outer_pt_index_in_contour_ = outer_.countPoints();
}

void PathStroker::addDegenerateLine(const QuadConstruct* quad_pts)
{
  const Point* quad = quad_pts->quad;
  Path* path = stroke_type_ == StrokeType::kOuter_StrokeType ? std::addressof(outer_)
                                                  : std::addressof(inner_);
  path->lineTo(quad[2].x, quad[2].y);
}

PathStroker::ReductionType PathStroker::CheckConicLinear(Conic const& conic,
                                                         Point* reduction)
{
  bool degenerate_AB = DegenerateVector(conic.pts[1] - conic.pts[0]);
  bool degenerate_BC = DegenerateVector(conic.pts[2] - conic.pts[1]);
  if (degenerate_AB & degenerate_BC) {
    return ReductionType::kPoint_ReductionType;
  }

  if (degenerate_AB | degenerate_BC) {
    return ReductionType::kLine_ReductionType;
  }

  if (!conic_in_line(conic)) {
    return ReductionType::kQuad_ReductionType;
  }

  float t = FindQuadMaxCurvature(conic.pts);
  if (t == 0) {
    return ReductionType::kLine_ReductionType;
  }

  conic.evalAt(t, reduction, nullptr);
  return ReductionType::kDegenerate_ReductionType;
}

PathStroker::ReductionType PathStroker::CheckCubicLinear(
    const Point* cubic, Point* redunction, const Point** tangent_pt_ptr)
{
  bool degenerate_AB = DegenerateVector(cubic[1] - cubic[0]);
  bool degenerate_BC = DegenerateVector(cubic[2] - cubic[1]);
  bool degenerate_CD = DegenerateVector(cubic[3] - cubic[2]);

  if (degenerate_AB & degenerate_BC & degenerate_CD) {
    return ReductionType::kPoint_ReductionType;
  }

  if (degenerate_AB + degenerate_BC + degenerate_CD == 2) {
    return ReductionType::kLine_ReductionType;
  }

  if (!cubic_in_line(cubic)) {
    *tangent_pt_ptr =
        degenerate_AB ? std::addressof(cubic[2]) : std::addressof(cubic[1]);
    return ReductionType::kQuad_ReductionType;
  }

  std::array<float, 3> t_values;
  int32_t count = FindCubicMaxCurvature(cubic, t_values.data());
  int32_t r_count = 0;
  for (int32_t index = 0; index < count; index++) {
    float t = t_values[index];
    if (0 >= t || t >= 1) {
      continue;
    }

    CubicCoeff::EvalCubicAt(cubic, t, std::addressof(redunction[r_count]),
                            nullptr, nullptr);
    if (redunction[r_count] != cubic[0] && redunction[r_count] != cubic[3]) {
      ++r_count;
    }
  }

  if (r_count == 0) {
    return ReductionType::kLine_ReductionType;
  }

  static_assert(kQuad_ReductionType + 1 == kDegenerate_ReductionType,
                "enum_out_of_whack");
  static_assert(kQuad_ReductionType + 2 == kDegenerate2_ReductionType,
                "enum_out_of_whack");
  static_assert(kQuad_ReductionType + 3 == kDegenerate3_ReductionType,
                "enum_out_of_whack");

  return static_cast<ReductionType>(kQuad_ReductionType + r_count);
}

PathStroker::ReductionType PathStroker::CheckQuadLinear(const Point quad[3],
                                                        Point* reduction)
{
  bool degenerate_AB = DegenerateVector(quad[1] - quad[0]);
  bool degenerate_BC = DegenerateVector(quad[2] - quad[1]);
  if (degenerate_AB & degenerate_BC) {
    return ReductionType::kPoint_ReductionType;
  }

  if (degenerate_AB | degenerate_BC) {
    return ReductionType::kLine_ReductionType;
  }

  if (!quad_in_line(quad)) {
    return ReductionType::kQuad_ReductionType;
  }

  float t = FindQuadMaxCurvature(quad);
  if (0 == t || 1 == t) {
    return ReductionType::kLine_ReductionType;
  }

  *reduction = QuadCoeff::EvalQuadAt({quad[0], quad[1], quad[2]}, t);
  return ReductionType::kDegenerate_ReductionType;
}

PathStroker::ResultType PathStroker::compareQuadConic(Conic const& conic,
                                                      QuadConstruct* quad_pts)
{
  this->conicQuadEnds(conic, quad_pts);
  ResultType result_type =
      this->intersectRay(quad_pts, IntersectRayType::kCtrlPt);
  if (result_type != ResultType::kQuad) {
    return result_type;
  }

  Point ray[2];
  this->conicPrepRay(conic, quad_pts->midT, std::addressof(ray[1]),
                     std::addressof(ray[0]), nullptr);
  return this->strokeCloseEnough(quad_pts->quad, ray, quad_pts);
}

PathStroker::ResultType PathStroker::compareQuadCubic(const Point* cubic,
                                                      QuadConstruct* quad_pts)
{
  this->cubicQuadEnds(cubic, quad_pts);
  ResultType result_type =
      this->intersectRay(quad_pts, IntersectRayType::kCtrlPt);
  if (result_type != ResultType::kQuad) {
    return result_type;
  }

  Point ray[2];
  this->cubicPerpRay(cubic, quad_pts->midT, std::addressof(ray[1]),
                     std::addressof(ray[0]), nullptr);
  return this->strokeCloseEnough(quad_pts->quad, ray, quad_pts);
}

PathStroker::ResultType PathStroker::compareQuadQuad(const Point quad[3],
                                                     QuadConstruct* quad_pts)
{
  if (!quad_pts->startSet) {
    Point quad_start_pt;
    this->quadPerpRay(quad, quad_pts->startT, std::addressof(quad_start_pt),
                      std::addressof(quad_pts->quad[0]),
                      std::addressof(quad_pts->tangentStart));
    quad_pts->startSet = true;
  }

  if (!quad_pts->endSet) {
    Point quad_end_pt;
    this->quadPerpRay(quad, quad_pts->endT, std::addressof(quad_end_pt),
                      std::addressof(quad_pts->quad[2]),
                      std::addressof(quad_pts->tangentEnd));
    quad_pts->endSet = true;
  }

  ResultType result_type =
      this->intersectRay(quad_pts, IntersectRayType::kCtrlPt);
  if (result_type != ResultType::kQuad) {
    return result_type;
  }

  // project a ray from the curve to the stroke
  Point ray[2];
  this->quadPerpRay(quad, quad_pts->midT, std::addressof(ray[1]),
                    std::addressof(ray[0]), nullptr);

  return this->strokeCloseEnough(quad_pts->quad, ray, quad_pts);
}

void PathStroker::conicPrepRay(Conic const& conic, float t, Point* t_pt,
                               Point* on_pt, Point* tangent) const
{
  Vector dxy;
  conic.evalAt(t, t_pt, std::addressof(dxy));
  if (dxy.x == 0 && dxy.y == 0) {
    dxy = conic.pts[2] - conic.pts[0];
  }

  this->setRayPts(*t_pt, std::addressof(dxy), on_pt, tangent);
}

void PathStroker::conicQuadEnds(Conic const& conic,
                                QuadConstruct* quad_pts) const
{
  if (!quad_pts->startSet) {
    Point conic_start_pt;
    this->conicPrepRay(conic, quad_pts->startT, std::addressof(conic_start_pt),
                       std::addressof(quad_pts->quad[0]),
                       std::addressof(quad_pts->tangentStart));
  }

  if (!quad_pts->endSet) {
    Point conic_end_pt;
    this->conicPrepRay(conic, quad_pts->endT, std::addressof(conic_end_pt),
                       std::addressof(quad_pts->quad[2]),
                       std::addressof(quad_pts->tangentEnd));
    quad_pts->endSet = true;
  }
}

bool PathStroker::conicStroke(Conic const& conic, QuadConstruct* quad_pts)
{
  ResultType result_type = this->compareQuadConic(conic, quad_pts);
  if (result_type == ResultType::kQuad) {
    const Point* stroke = quad_pts->quad;
    Path* path = stroke_type_ == StrokeType::kOuter_StrokeType ? std::addressof(outer_)
                                                    : std::addressof(inner_);
    path->quadTo(stroke[1].x, stroke[1].y, stroke[2].x, stroke[2].y);
    return true;
  }

  if (result_type == ResultType::kDegenerate) {
    addDegenerateLine(quad_pts);
    return true;
  }

  if (++recursion_depth_ > kRecursiveLimit[kConic_RecursiveLimit]) {
    return false;
  }

  QuadConstruct half;
  half.initWithStart(quad_pts);
  if (!this->conicStroke(conic, std::addressof(half))) {
    return false;
  }
  half.initWithEnd(quad_pts);
  if (!this->conicStroke(conic, std::addressof(half))) {
    return false;
  }
  --recursion_depth_;
  return true;
}

bool PathStroker::cubicMidOnLine(const Point* cubic,
                                 const QuadConstruct* quad_pts) const
{
  Point stroke_mid;
  this->cubicQuadMid(cubic, quad_pts, std::addressof(stroke_mid));
  float dist = pt_to_line(stroke_mid, quad_pts->quad[0], quad_pts->quad[2]);
  return dist < inv_res_scale_squared_;
}

void PathStroker::cubicPerpRay(const Point* cubic, float t, Point* t_pt,
                               Point* on_pt, Point* tangent) const
{
  Vector dxy;
  std::array<Point, 7> chopped;
  CubicCoeff::EvalCubicAt(cubic, t, t_pt, std::addressof(dxy), nullptr);
  if (dxy.x == 0 && dxy.y == 0) {
    const Point* c_pts = cubic;
    if (FloatNearlyZero(t)) {
      dxy = cubic[2] - cubic[0];
    }
    else if (FloatNearlyZero(1 - t)) {
      dxy = cubic[3] - cubic[1];
    }
    else {
      CubicCoeff::ChopCubicAt(cubic, chopped.data(), t);
      dxy = chopped[3] - chopped[2];
      if (dxy.x == 0 && dxy.y == 0) {
        dxy = chopped[3] - chopped[1];
        c_pts = chopped.data();
      }
    }

    if (dxy.x == 0 && dxy.y == 0) {
      dxy = c_pts[3] - c_pts[0];
    }
  }
  setRayPts(*t_pt, std::addressof(dxy), on_pt, tangent);
}

void PathStroker::cubicQuadMid(const Point* cubic,
                               const QuadConstruct* quad_pts, Point* mid) const
{
  Point cubic_mid_pt;
  this->cubicPerpRay(cubic, quad_pts->midT, std::addressof(cubic_mid_pt), mid,
                     nullptr);
}

void PathStroker::cubicQuadEnds(const Point* cubic, QuadConstruct* quad_pts)
{
  if (!quad_pts->startSet) {
    Point cubic_start_pt;
    this->cubicPerpRay(cubic, quad_pts->startT, std::addressof(cubic_start_pt),
                       std::addressof(quad_pts->quad[0]),
                       std::addressof(quad_pts->tangentStart));
    quad_pts->startSet = true;
  }

  if (!quad_pts->endSet) {
    Point cubic_end_pt;
    this->cubicPerpRay(cubic, quad_pts->endT, std::addressof(cubic_end_pt),
                       std::addressof(quad_pts->quad[2]),
                       std::addressof(quad_pts->tangentEnd));
    quad_pts->endSet = true;
  }
}

bool PathStroker::cubicStroke(const Point cubic[4], QuadConstruct* quad_pts)
{
  if (found_tangents_) {
    ResultType result_type = this->tangentsMeet(cubic, quad_pts);
    if (result_type != ResultType::kQuad) {
      if ((result_type == ResultType::kDegenerate ||
           PointsWithInDist(quad_pts->quad[0], quad_pts->quad[2],
                            inv_res_scale_)) &&
          cubicMidOnLine(cubic, quad_pts)) {
        addDegenerateLine(quad_pts);
        return true;
      }
    }
    else {
      found_tangents_ = true;
    }
  }

  if (found_tangents_) {
    ResultType result_type = this->compareQuadCubic(cubic, quad_pts);
    if (result_type == ResultType::kQuad) {
      Path* path = stroke_type_ == StrokeType::kOuter_StrokeType ? std::addressof(outer_)
                                                      : std::addressof(inner_);
      const Point* stroke = quad_pts->quad;
      path->quadTo(stroke[1].x, stroke[1].y, stroke[2].x, stroke[2].y);
      return true;
    }

    if (result_type == ResultType::kDegenerate) {
      if (!quad_pts->opposieTangents) {
        addDegenerateLine(quad_pts);
        return true;
      }
    }
  }

  if (glm::isinf(quad_pts->quad[2].x) || glm::isinf(quad_pts->quad[2].y)) {
    return false;
  }

  if (++recursion_depth_ > kRecursiveLimit[found_tangents_]) {
    return false;
  }

  auto half = std::make_unique<QuadConstruct>();
  if (!half->initWithStart(quad_pts)) {
    addDegenerateLine(quad_pts);
    --recursion_depth_;
    return true;
  }

  if (!this->cubicStroke(cubic, half.get())) {
    return false;
  }

  if (!half->initWithEnd(quad_pts)) {
    addDegenerateLine(quad_pts);
    --recursion_depth_;
    return true;
  }

  if (!this->cubicStroke(cubic, half.get())) {
    return false;
  }

  --recursion_depth_;
  return true;
}

void PathStroker::quadPerpRay(const Point* quad, float t, Point* t_pt,
                              Point* on_pt, Point* tangent) const
{
  Vector dxy;
  QuadCoeff::EvalQuadAt({quad[0], quad[1], quad[2]}, t, t_pt,
                        std::addressof(dxy));
  if (dxy.x == 0 && dxy.y == 0) {
    dxy = quad[2] - quad[0];
  }

  setRayPts(*t_pt, std::addressof(dxy), on_pt, tangent);
}

bool PathStroker::quadStroke(const Point quad[3], QuadConstruct* quad_pts)
{
  ResultType result_type = this->compareQuadQuad(quad, quad_pts);

  if (result_type == ResultType::kQuad) {
    const Point* stroke = quad_pts->quad;
    Path* path = stroke_type_ == StrokeType::kOuter_StrokeType ? std::addressof(outer_)
                                                    : std::addressof(inner_);
    path->quadTo(stroke[1].x, stroke[1].y, stroke[2].x, stroke[2].y);
    return true;
  }

  if (result_type == ResultType::kDegenerate) {
    addDegenerateLine(quad_pts);
    return true;
  }

  if (++recursion_depth_ > kRecursiveLimit[kQuad_RecursiveLimit]) {
    return false;
  }

  auto half = std::make_unique<QuadConstruct>();
  half->initWithStart(quad_pts);
  if (!this->quadStroke(quad, half.get())) {
    return false;
  }
  half->initWithEnd(quad_pts);
  if (!this->quadStroke(quad, half.get())) {
    return false;
  }

  --recursion_depth_;
  return true;
}

void PathStroker::setConicEndNormal(Conic const& conic, Vector const& normalAB,
                                    Vector const& unitNormalAB,
                                    Vector* normalBC, Vector* unitNormalBC)
{
  setQuadEndNormal(conic.pts, normalAB, unitNormalAB, normalBC, unitNormalBC);
}

void PathStroker::setCubicEndNormal(const Point* cubic, Vector const& normalAB,
                                    Vector const& unitNormalAB,
                                    Vector* normalCD, Vector* unitNormalCD)
{
  Vector ab = cubic[1] - cubic[0];
  Vector cd = cubic[3] - cubic[2];

  bool degenerate_AB = DegenerateVector(ab);
  bool degenerate_CD = DegenerateVector(cd);

  if (degenerate_AB && degenerate_CD) {
    goto DEGENERATE_NORMAL;
  }

  if (degenerate_AB) {
    ab = cubic[2] - cubic[0];
    degenerate_AB = DegenerateVector(ab);
  }

  if (degenerate_CD) {
    cd = cubic[3] - cubic[1];
    degenerate_CD = DegenerateVector(cd);
  }

  if (degenerate_AB || degenerate_CD) {
  DEGENERATE_NORMAL:
    *normalCD = normalAB;
    *unitNormalCD = unitNormalAB;
    return;
  }

  set_normal_unitnormal(cd, radius_, normalCD, unitNormalCD);
}

PathStroker::ResultType PathStroker::intersectRay(
    QuadConstruct* quad_pts, IntersectRayType intersect_ray_type) const
{
  Point const& start = quad_pts->quad[0];
  Point const& end = quad_pts->quad[2];

  Vector a_len = quad_pts->tangentStart - start;
  Vector b_len = quad_pts->tangentEnd - end;

  float denom = CrossProduct(a_len, b_len);

  if (denom == 0 || glm::isinf(denom)) {
    quad_pts->opposieTangents = glm::dot(a_len, b_len) < 0;
    return ResultType::kDegenerate;
  }

  quad_pts->opposieTangents = false;
  Vector ab0 = start - end;
  float numberA = CrossProduct(b_len, ab0);
  float numberB = CrossProduct(a_len, ab0);

  if ((numberA >= 0) == (numberB >= 0)) {
    // if the prependicular distances from the quad points to the opposite
    // tangent line are small, a straight line is good enough
    float dist1 = pt_to_line(start, end, quad_pts->tangentEnd);
    float dist2 = pt_to_line(end, start, quad_pts->tangentStart);
    if (glm::max(dist1, dist2) <= inv_res_scale_squared_) {
      return ResultType::kDegenerate;
    }
    return ResultType::kSplit;
  }
  // check to see if the denominator is teeny relative to the numerator if the
  // offset by one will be lost, the ratio is too large
  numberA /= denom;
  bool valid_divided = numberA > numberA - 1;
  if (valid_divided) {
    if (intersect_ray_type == IntersectRayType::kCtrlPt) {
      Point* ctrl_pt = std::addressof(quad_pts->quad[1]);
      PointSet(*ctrl_pt,
               start.x * (1 - numberA) + quad_pts->tangentStart.x * numberA,
               start.y * (1 - numberA) + quad_pts->tangentStart.y * numberA);
    }
    return ResultType::kQuad;
  }

  quad_pts->opposieTangents = glm::dot(a_len, b_len) < 0;
  // if the lines are parallel, straight line is good enough
  return ResultType::kDegenerate;
}
}  // namespace skity
