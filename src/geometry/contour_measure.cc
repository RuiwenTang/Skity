#include "src/geometry/contour_measure.hpp"

#include <array>
#include <cassert>
#include <tuple>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/graphic/path_priv.hpp"

namespace skity {

#define CHEAP_DIST_LIMIT (Float1 / 2)
#define kMaxTValue 0x3FFFFFFF

static bool cheap_dist_exceeds_limit(Point const& pt, float x, float y,
                                     float to_lerance) {
  float dist = std::max(std::abs(x - pt.x), std::abs(y - pt.y));

  return dist > to_lerance;
}

static inline int tspan_big_enough(int tspan) {
  assert(tspan <= kMaxTValue);
  return tspan >> 10;
}

static bool quad_too_curvy(const Point pts[3], float to_lerance) {
  // diff = (a/4 + b/2 + c/4) - (a/2 + c/2)
  // diff = -a/4 + b/2 - c/4
  float dx = SkityFloatHalf(pts[1].x) -
             SkityFloatHalf(SkityFloatHalf(pts[0].x + pts[2].x));
  float dy = SkityFloatHalf(pts[1].y) -
             SkityFloatHalf(SkityFloatHalf(pts[0].y + pts[2].y));

  float dist = std::max(std::abs(dx), std::abs(dy));

  return dist > to_lerance;
}

static bool conic_too_curvy(Point const& firstPt, Point const& midTPt,
                            Point const& lastPt, float to_lerance) {
  Point midEnds = firstPt + lastPt;
  midEnds *= 0.5f;
  Vector dxy = midTPt - midEnds;
  float dist = std::max(std::abs(dxy.x), std::abs(dxy.y));
  return dist > to_lerance;
}

static bool cubic_too_curvy(const Point pts[4], float to_lerance) {
  return cheap_dist_exceeds_limit(
             pts[1], FloatInterp(pts[0].x, pts[3].x, Float1 / 3),
             FloatInterp(pts[0].y, pts[3].y, Float1 / 3), to_lerance) ||
         cheap_dist_exceeds_limit(
             pts[2], FloatInterp(pts[0].x, pts[3].x, Float1 * 2 / 3),
             FloatInterp(pts[0].y, pts[3].y, Float1 * 2 / 3), to_lerance);
}

constexpr static inline float tValue2Float(int t) {
  assert((unsigned)t <= kMaxTValue);
  const float kMaxTReciprocal = 1.f / (float)kMaxTValue;
  return t * kMaxTReciprocal;
}

static void compute_pos_tan(const Point pts[], unsigned segType, float t,
                            Point* pos, Vector* tangent) {
  switch (segType) {
    case ContourMeasure::kLine_SegType:
      if (pos) {
        PointSet(pos[0], FloatInterp(pts[0].x, pts[1].x, t),
                 FloatInterp(pts[0].y, pts[1].y, t));
      }
      if (tangent) {
        *tangent = Vector{glm::normalize(Vec2{pts[1] - pts[0]}), 0.f, 0.f};
      }
      break;
    case ContourMeasure::kQuad_SegType:
      QuadCoeff::EvalQuadAt({pts[0], pts[1], pts[2]}, t, pos, tangent);
      if (tangent) {
        *tangent = Vector{glm::normalize(Vec2(tangent->x, tangent->y)), 0, 0};
      }
      break;
    case ContourMeasure::kConic_SegType:
      Conic{pts[0], pts[2], pts[3], pts[1].x}.evalAt(t, pos, tangent);
      if (tangent) {
        *tangent = Vector{glm::normalize(Vec2(tangent->x, tangent->y)), 0, 0};
      }
      break;
    case ContourMeasure::kCubic_SegType:
      CubicCoeff::EvalCubicAt(pts, t, pos, tangent, nullptr);
      if (tangent) {
        *tangent = Vector{glm::normalize(Vec2(tangent->x, tangent->y)), 0, 0};
      }
      break;
    default:
      assert(false);
  }
}

static void contour_measure_seg_to(const Point pts[], unsigned segType,
                                   float startT, float stopT, Path* dst) {
  assert(startT >= 0 && startT <= Float1);
  assert(stopT >= 0 && stopT <= Float1);
  assert(startT <= stopT);

  if (startT == stopT) {
    if (!dst->isEmpty()) {
      /**
       *  if the dash as a zero-length on segment, add a corresponding
       *  zero-length line. The stroke code will add end caps to zero length
       *  lines as appropriate
       */
      Point lastPt;
      assert(dst->getLastPt(&lastPt));
      dst->lineTo(lastPt);
    }
    return;
  }

  std::array<Point, 7> tmp0 = {};
  std::array<Point, 7> tmp1 = {};

  switch (segType) {
    case ContourMeasure::kLine_SegType:
      if (Float1 == stopT) {
        dst->lineTo(pts[1]);
      } else {
        dst->lineTo(FloatInterp(pts[0].x, pts[1].x, stopT),
                    FloatInterp(pts[0].y, pts[1].y, stopT));
      }
      break;
    case ContourMeasure::kQuad_SegType:
      if (0 == startT) {
        if (Float1 == stopT) {
          dst->quadTo(pts[1], pts[2]);
        } else {
          QuadCoeff::ChopQuadAt(pts, tmp0.data(), stopT);
          dst->quadTo(tmp0[1], tmp0[2]);
        }
      } else {
        QuadCoeff::ChopQuadAt(pts, tmp0.data(), startT);
        if (Float1 == stopT) {
          dst->quadTo(tmp0[3], tmp0[4]);
        } else {
          QuadCoeff::ChopQuadAt(&tmp0[2], tmp1.data(),
                                (stopT - startT) / (1.f - startT));
          dst->quadTo(tmp1[1], tmp1[2]);
        }
      }
      break;
    case ContourMeasure::kConic_SegType: {
      Conic conic{pts[0], pts[2], pts[3], pts[1].x};
      if (0.f == startT) {
        if (Float1 == stopT) {
          dst->conicTo(conic.pts[1], conic.pts[2], conic.w);
        } else {
          std::array<Conic, 2> tmp;
          if (conic.chopAt(stopT, tmp.data())) {
            dst->conicTo(tmp[0].pts[1], tmp[0].pts[2], tmp[0].w);
          }
        }
      } else {
        if (Float1 == stopT) {
          std::array<Conic, 2> tmp;
          if (conic.chopAt(startT, tmp.data())) {
            dst->conicTo(tmp[1].pts[1], tmp[1].pts[2], tmp[1].w);
          }
        } else {
          Conic tmp;
          conic.chopAt(startT, stopT, &tmp);
          dst->conicTo(tmp.pts[1], tmp.pts[2], tmp.w);
        }
      }
    } break;
    case ContourMeasure::kCubic_SegType:
      if (0 == startT) {
        if (Float1 == stopT) {
          dst->cubicTo(pts[1], pts[2], pts[3]);
        } else {
          CubicCoeff::ChopCubicAt(pts, tmp0.data(), stopT);
          dst->cubicTo(tmp0[1], tmp0[2], tmp0[3]);
        }
      } else {
        CubicCoeff::ChopCubicAt(pts, tmp0.data(), startT);
        if (Float1 == stopT) {
          dst->cubicTo(tmp0[4], tmp0[5], tmp0[6]);
        } else {
          CubicCoeff::ChopCubicAt(&tmp0[3], tmp1.data(),
                                  (stopT - startT) / (1.f - startT));
          dst->cubicTo(tmp1[1], tmp1[2], tmp1[3]);
        }
      }
      break;
    default:
      // should never reach here
      assert(false);
      break;
  }
}

//--------------------------------- Impl --------------------

class ContourMeasureIter::Impl {
 public:
  Impl(Path const& path, bool force_closed, float resScale)
      : path_(path),
        iter_(PathPriv::Iterate(path_).begin()),
        to_lerance_(CHEAP_DIST_LIMIT * FloatInvert(resScale)),
        force_closed_(force_closed) {}
  bool hasNextSegments() const {
    return iter_ != PathPriv::Iterate(path_).end();
  }

  ContourMeasure* buildSegments();

 private:
  float ComputeLineSeg(Point p0, Point p1, float distance, uint32_t pt_index);

  float ComputedQuadSegs(const Point pts[3], float distance, int mint, int maxt,
                         uint32_t pt_index);

  float ComputeConicSegs(Conic const& conic, float distance, int mint,
                         Point const& min_pt, int maxt, Point const& max_pt,
                         uint32_t pt_index);

  float ComputeCubicSegs(const Point pts[4], float distance, int mint, int maxt,
                         uint32_t pt_index);

 private:
  Path path_;
  Path::RangeIter iter_;
  float to_lerance_;
  bool force_closed_;
  std::vector<ContourMeasure::Segment> segments_;
  std::vector<Point> pts_;
};

float ContourMeasureIter::Impl::ComputeLineSeg(Point p0, Point p1,
                                               float distance,
                                               uint32_t pt_index) {
  float d = PointDistance(p0, p1);
  assert(d >= 0);
  float prevD = distance;
  distance += d;
  if (distance > prevD) {
    assert(pt_index < pts_.size());
    ContourMeasure::Segment seg{};
    seg.distance = distance;
    seg.pt_index = pt_index;
    seg.type = ContourMeasure::kLine_SegType;
    seg.t_value = kMaxTValue;
    segments_.emplace_back(seg);
  }
  return distance;
}

float ContourMeasureIter::Impl::ComputedQuadSegs(const Point pts[3],
                                                 float distance, int mint,
                                                 int maxt, uint32_t pt_index) {
  if (tspan_big_enough(maxt - mint) && quad_too_curvy(pts, to_lerance_)) {
    std::array<Point, 3> tmp1{};
    std::array<Point, 3> tmp2{};
    int halft = (mint + maxt) >> 1;

    SubDividedQuad(pts, tmp1.data(), tmp2.data());
    distance =
        this->ComputedQuadSegs(tmp1.data(), distance, mint, halft, pt_index);
    distance =
        this->ComputedQuadSegs(tmp2.data(), distance, halft, maxt, pt_index);
  } else {
    float d = PointDistance(pts[0], pts[2]);
    float prevD = distance;
    distance += d;
    if (distance > prevD) {
      assert(pt_index < pts_.size());
      ContourMeasure::Segment seg{};
      seg.distance = distance;
      seg.pt_index = pt_index;
      seg.type = ContourMeasure::kQuad_SegType;
      seg.t_value = maxt;
      segments_.emplace_back(seg);
    }
  }
  return distance;
}

float ContourMeasureIter::Impl::ComputeConicSegs(Conic const& conic,
                                                 float distance, int mint,
                                                 Point const& min_pt, int maxt,
                                                 Point const& max_pt,
                                                 uint32_t pt_index) {
  int32_t halft = (mint + maxt) >> 1;
  Point halfPt = conic.evalAt(tValue2Float(halft));
  if (!PointIsFinite(halfPt)) {
    return distance;
  }

  if (tspan_big_enough(maxt - mint) &&
      conic_too_curvy(min_pt, halfPt, max_pt, to_lerance_)) {
    distance = this->ComputeConicSegs(conic, distance, mint, min_pt, halft,
                                      halfPt, pt_index);
    distance = this->ComputeConicSegs(conic, distance, halft, halfPt, maxt,
                                      max_pt, pt_index);
  } else {
    float d = PointDistance(min_pt, max_pt);
    float prevD = distance;
    distance += d;
    if (distance > prevD) {
      assert(pt_index < pts_.size());
      ContourMeasure::Segment seg{};
      seg.distance = distance;
      seg.pt_index = pt_index;
      seg.type = ContourMeasure::kConic_SegType;
      seg.t_value = maxt;
      segments_.emplace_back(seg);
    }
  }

  return distance;
}

float ContourMeasureIter::Impl::ComputeCubicSegs(const Point pts[4],
                                                 float distance, int mint,
                                                 int maxt, uint32_t pt_index) {
  if (tspan_big_enough(maxt - mint) && cubic_too_curvy(pts, to_lerance_)) {
    std::array<Point, 4> tmp1{};
    std::array<Point, 4> tmp2{};
    int halft = (mint + maxt) >> 1;

    SubDividedCubic(pts, tmp1.data(), tmp2.data());
    distance =
        this->ComputeCubicSegs(tmp1.data(), distance, mint, halft, pt_index);
    distance =
        this->ComputeCubicSegs(tmp2.data(), distance, halft, maxt, pt_index);
  } else {
    float d = PointDistance(pts[0], pts[3]);
    float prevD = distance;
    distance += d;
    if (distance > prevD) {
      assert(pt_index < pts_.size());

      ContourMeasure::Segment seg{};
      seg.distance = distance;
      seg.pt_index = pt_index;
      seg.type = ContourMeasure::kCubic_SegType;
      seg.t_value = maxt;
      segments_.emplace_back(seg);
    }
  }

  return distance;
}

ContourMeasure* ContourMeasureIter::Impl::buildSegments() {
  int pt_index = -1;
  float distance = 0;
  bool have_seen_close = force_closed_;
  bool have_seen_move_to = false;

  /*  Note:
   *  as we accumulate distance, we have to check that the result of +=
   *  actually made it larger, since a very small delta might be > 0, but
   *  still have no effect on distance (if distance >>> delta).
   *
   *  We do this check below, and in compute_quad_segs and compute_cubic_segs
   */

  segments_.clear();
  pts_.clear();

  auto end = PathPriv::Iterate(path_).end();
  for (; iter_ != end; ++iter_) {
    auto ret = *iter_;
    auto verb = std::get<0>(ret);
    auto pts = std::get<1>(ret);
    auto w = std::get<2>(ret);
    if (have_seen_move_to && verb == Path::Verb::kMove) {
      break;
    }

    switch (verb) {
      case Path::Verb::kMove:
        pt_index += 1;
        pts_.emplace_back(pts[0]);
        assert(!have_seen_move_to);
        have_seen_move_to = true;
        break;
      case Path::Verb::kLine: {
        assert(have_seen_move_to);
        float prevD = distance;
        distance = this->ComputeLineSeg(pts[0], pts[1], distance, pt_index);
        if (distance > prevD) {
          pts_.emplace_back(pts[1]);
          pt_index++;
        }
      } break;
      case Path::Verb::kQuad: {
        assert(have_seen_move_to);
        float prevD = distance;
        distance =
            this->ComputedQuadSegs(pts, distance, 0, kMaxTValue, pt_index);
        if (distance > prevD) {
          pts_.emplace_back(pts[1]);
          pts_.emplace_back(pts[2]);
          pt_index += 2;
        }
      } break;
      case Path::Verb::kConic: {
        assert(have_seen_move_to);
        Conic conic{pts, *w};
        float prevD = distance;
        distance = this->ComputeConicSegs(conic, distance, 0, conic.pts[0],
                                          kMaxTValue, conic.pts[2], pt_index);
        if (distance > prevD) {
          // we store the conic weight in our next point, followed by the last
          // 2 pts thus to reconstitue a conic, you'd need to say
          // SkConic(pts[0], pts[2], pts[3], weight = pts[1].x)
          pts_.emplace_back(Point{conic.w, 0, 0, 0});
          pts_.emplace_back(pts[1]);
          pts_.emplace_back(pts[2]);
          pt_index += 3;
        }
      } break;
      case Path::Verb::kCubic: {
        assert(have_seen_move_to);
        float prevD = distance;
        distance =
            this->ComputeCubicSegs(pts, distance, 0, kMaxTValue, pt_index);
        if (distance > prevD) {
          pts_.emplace_back(pts[1]);
          pts_.emplace_back(pts[2]);
          pts_.emplace_back(pts[3]);
          pt_index += 3;
        }
      } break;
      case Path::Verb::kClose:
        have_seen_close = true;
        break;
      case Path::Verb::kDone:
        break;
    }
  }

  if (!FloatIsFinite(distance)) {
    return nullptr;
  }

  if (segments_.empty()) {
    return nullptr;
  }

  if (have_seen_close) {
    float prevD = distance;
    Point firstPt = pts_[0];
    distance =
        this->ComputeLineSeg(pts_[pt_index], firstPt, distance, pt_index);
    if (distance > prevD) {
      pts_.emplace_back(firstPt);
    }
  }

  return new ContourMeasure(std::move(segments_), std::move(pts_), distance,
                            have_seen_close);
}

//--------------------------------- ContourMeasureIter --------------------

ContourMeasureIter::ContourMeasureIter() = default;

ContourMeasureIter::ContourMeasureIter(Path const& path, bool forceClosed,
                                       float resScale) {
  this->reset(path, forceClosed, resScale);
}

ContourMeasureIter::~ContourMeasureIter() = default;

void ContourMeasureIter::reset(Path const& path, bool forceClosed,
                               float resScale) {
  if (path.isFinite()) {
    impl_ = std::make_unique<Impl>(path, forceClosed, resScale);
  } else {
    impl_.reset();
  }
}

std::shared_ptr<ContourMeasure> ContourMeasureIter::next() {
  if (!impl_) {
    return nullptr;
  }

  while (impl_->hasNextSegments()) {
    auto cm = impl_->buildSegments();
    if (cm) {
      return std::shared_ptr<ContourMeasure>(cm);
    }
  }

  return nullptr;
}

//--------------------------------- ContourMeasure --------------------

template <typename T, typename K>
int TKSearch(const T base[], int count, const K& key) {
  assert(count >= 0);
  if (count <= 0) {
    return ~0;
  }

  assert(base != nullptr);
  unsigned lo = 0;
  unsigned hi = count - 1;
  while (lo < hi) {
    unsigned mid = (lo + hi) >> 1;
    if (base[mid].distance < key) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }

  if (base[hi].distance < key) {
    hi += 1;
    hi = ~hi;
  } else if (key < base[hi].distance) {
    hi = ~hi;
  }

  return hi;
}

ContourMeasure::ContourMeasure(std::vector<Segment>&& segs,
                               std::vector<Point>&& pts, float length,
                               bool isClosed)
    : segments_(std::move(segs)),
      pts_(std::move(pts)),
      length_(length),
      is_closed_(isClosed) {}

bool ContourMeasure::getPosTan(float distance, Point* position,
                               Vector* tangent) const {
  if (FloatIsNan(distance)) {
    return false;
  }

  float length = this->length();
  assert(length > 0 && !segments_.empty());

  if (distance < 0) {
    distance = 0;
  } else if (distance > length) {
    distance = length;
  }

  float t;

  const Segment* seg = this->distanceToSegment(distance, &t);
  if (FloatIsNan(t)) {
    return false;
  }

  assert((unsigned)seg->pt_index < (unsigned)pts_.size());
  compute_pos_tan(&pts_[seg->pt_index], seg->type, t, position, tangent);
  return true;
}

bool ContourMeasure::getSegment(float startD, float stopD, Path* dst,
                                bool startWithMoveTo) const {
  assert(dst);

  float length = this->length();

  if (startD < 0) {
    startD = 0;
  }
  if (stopD > length) {
    stopD = length;
  }

  if (!(startD <= stopD)) {  // catch NaN values as well
    return false;
  }

  if (segments_.size() == 0) {
    return false;
  }

  Point p;
  float startT, stopT;
  const Segment* seg = this->distanceToSegment(startD, &startT);
  if (!FloatIsFinite(startT)) {
    return false;
  }

  const Segment* stopSeg = this->distanceToSegment(stopD, &stopT);
  if (!FloatIsFinite(stopT)) {
    return false;
  }

  assert(seg <= stopSeg);

  if (startWithMoveTo) {
    compute_pos_tan(&pts_[seg->pt_index], seg->type, startT, &p, nullptr);
    dst->moveTo(p);
  }

  if (seg->pt_index == stopSeg->pt_index) {
    contour_measure_seg_to(&pts_[seg->pt_index], seg->type, startT, stopT, dst);
  } else {
    do {
      contour_measure_seg_to(&pts_[seg->pt_index], seg->type, startT, Float1,
                             dst);
      seg = ContourMeasure::Segment::Next(seg);
      startT = 0;
    } while (seg->pt_index < stopSeg->pt_index);
    contour_measure_seg_to(&pts_[seg->pt_index], seg->type, 0, stopT, dst);
  }

  return true;
}

float ContourMeasure::Segment::getScalarT() const {
  return tValue2Float(t_value);
}

const ContourMeasure::Segment* ContourMeasure::distanceToSegment(
    float distance, float* t) const {
  float length = this->length();
  assert(distance >= 0 && distance <= length);

  const Segment* seg = segments_.data();
  int count = segments_.size();

  int index = TKSearch<Segment, float>(seg, count, distance);
  index ^= (index >> 31);
  seg = &seg[index];

  // now interpolate t-values with prev segment (if possible)
  float startT = 0, startD = 0;
  // check if the prev segment is legal, and references the same set of points
  if (index > 0) {
    startD = seg[-1].distance;
    if (seg[-1].pt_index == seg->pt_index) {
      assert(seg[-1].type == seg->type);
      startT = seg[-1].getScalarT();
    }
  }
  auto v = seg->getScalarT();
  assert(seg->getScalarT() > startT);
  assert(distance >= startD);
  assert(seg->distance > startD);

  *t = startT + (seg->getScalarT() - startT) * (distance - startD) /
                    (seg->distance - startD);
  return seg;
}

}  // namespace skity