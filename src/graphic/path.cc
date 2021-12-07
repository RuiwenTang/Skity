
#include <array>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/graphic/path.hpp>
#include <sstream>

#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/logging.hpp"

namespace skity {

template <size_t N>
class Path_PointIterator {
 public:
  using Array = std::array<Point, N>;

  Path_PointIterator(Path::Direction dir, size_t startIndex)
      : current_(startIndex % N),
        advance_(dir == Path::Direction::kCW ? 1 : N - 1) {}

  const Point& current() const {
    assert(current_ < N);
    return pts[current_];
  }

  const Point& next() {
    current_ = (current_ + advance_) % N;
    return this->current();
  }

 protected:
  Array pts;

 private:
  size_t current_;
  size_t advance_;
};

class Path_RectPointIterator : public Path_PointIterator<4> {
 public:
  Path_RectPointIterator(const Rect& rect, Path::Direction dir,
                         size_t startIndex)
      : Path_PointIterator<4>(dir, startIndex) {
    pts[0] = Point{rect.left(), rect.top(), 0, 1};
    pts[1] = Point{rect.right(), rect.top(), 0, 1};
    pts[2] = Point{rect.right(), rect.bottom(), 0, 1};
    pts[3] = Point{rect.left(), rect.bottom(), 0, 1};
  }
};

class Path_OvalPointIterator : public Path_PointIterator<4> {
 public:
  Path_OvalPointIterator(const Rect& oval, Path::Direction dir,
                         size_t startIndex)
      : Path_PointIterator<4>(dir, startIndex) {
    const float cx = oval.centerX();
    const float cy = oval.centerY();

    pts[0] = Point{cx, oval.top(), 0, 1};
    pts[1] = Point{oval.right(), cy, 0, 1};
    pts[2] = Point{cx, oval.bottom(), 0, 1};
    pts[3] = Point{oval.left(), cy, 0, 1};
  }
};

class Path_RRectPointIterator : public Path_PointIterator<8> {
 public:
  Path_RRectPointIterator(RRect const& rrect, Path::Direction dir,
                          uint32_t start_index)
      : Path_PointIterator(dir, start_index) {
    Rect const& bounds = rrect.getBounds();
    float L = bounds.left();
    float T = bounds.top();
    float R = bounds.right();
    float B = bounds.bottom();

    pts[0] = Point{L + rrect.radii(RRect::kUpperLeft).x, T, 0, 1.f};
    pts[1] = Point{R - rrect.radii(RRect::kUpperRight).x, T, 0, 1.f};
    pts[2] = Point{R, T + rrect.radii(RRect::kUpperRight).y, 0, 1.f};
    pts[3] = Point{R, B - rrect.radii(RRect::kLowerRight).y, 0, 1.f};
    pts[4] = Point{R - rrect.radii(RRect::kLowerRight).x, B, 0, 1.f};
    pts[5] = Point{L + rrect.radii(RRect::kLowerLeft).x, B, 0, 1.f};
    pts[6] = Point{L, B - rrect.radii(RRect::kLowerLeft).y, 0, 1.f};
    pts[7] = Point{L, T + rrect.radii(RRect::kUpperLeft).y, 0, 1.f};
  }
};

class AutoDisableDirectionCheck {
 public:
  explicit AutoDisableDirectionCheck(Path* p)
      : path{p}, saved{p->getFirstDirection()} {}
  ~AutoDisableDirectionCheck() { path->setFirstDirection(saved); }

 private:
  Path* path;
  Path::Direction saved;
};

class AutoPathBoundsUpdate {
 public:
  AutoPathBoundsUpdate(Path* p, const Rect& r) : path{p}, rect{r} {
    rect.sort();
    has_valid_bounds = path->isFinite();
    empty = path->isEmpty();
    if (has_valid_bounds && !empty) {
      JoinNoEmptyChecks(std::addressof(rect), path->getBounds());
    }
    degenerate = is_degenerate(*path);
  }

  ~AutoPathBoundsUpdate() {
    if ((this->empty || has_valid_bounds) && rect.isFinite()) {
      // TODO path->setBounds(rect);
    }
  }

 private:
  static void JoinNoEmptyChecks(Rect* dst, const Rect& src) {
    float s_left = src.left();
    float s_top = src.top();
    float s_right = src.right();
    float s_bottom = src.bottom();
    float d_left = dst->left();
    float d_top = dst->top();
    float d_right = dst->right();
    float d_bottom = dst->bottom();

    dst->setLTRB(glm::min(s_left, d_left), glm::min(s_top, d_top),
                 glm::min(d_right, s_right), glm::min(d_bottom, s_bottom));
  }

  static bool is_degenerate(const Path& path) { return path.countVerbs() <= 1; }

 private:
  Path* path;
  Rect rect;
  bool has_valid_bounds;
  bool degenerate;
  bool empty;
};

Path::Iter::Iter()
    : pts_{nullptr},
      verbs_{nullptr},
      verb_stop_{nullptr},
      conic_weights_{nullptr},
      force_close_(false),
      need_close_(false),
      close_line_(false),
      move_to_(),
      last_pt_(),
      segment_state_(SegmentState::kEmptyContour) {}

Path::Iter::Iter(const Path& path, bool forceClose) : Iter() {
  this->setPath(path, forceClose);
}

void Path::Iter::setPath(Path const& path, bool forceClose) {
  pts_ = path.points_.data();
  verbs_ = path.verbs_.data();
  verb_stop_ = path.verbs_.data() + path.countVerbs();
  conic_weights_ = path.conic_weights_.data();
  if (conic_weights_) {
    conic_weights_ -= 1;
  }

  force_close_ = forceClose;
  need_close_ = false;
  segment_state_ = SegmentState::kEmptyContour;
}

Path::Verb Path::Iter::next(Point pts[4]) {
  if (verbs_ == verb_stop_) {
    // Close the curve if requested and if there is some curve to close
    if (need_close_ && segment_state_ == SegmentState::kAfterPrimitive) {
      if (Verb::kLine == this->autoClose(pts)) {
        return Verb::kLine;
      }
      need_close_ = false;
      return Verb::kClose;
    }
    return Verb::kDone;
  }

  Verb verb = *verbs_++;
  const Point* src_pts = pts_;
  Point* p_pts = pts;

  switch (verb) {
    case Verb::kMove:
      if (need_close_) {
        verbs_--;  // move back one verb
        verb = this->autoClose(p_pts);
        if (verb == Verb::kClose) {
          need_close_ = false;
        }
        return verb;
      }
      if (verbs_ == verb_stop_) {
        return Verb::kDone;
      }
      move_to_ = *src_pts;
      p_pts[0] = *src_pts;
      src_pts += 1;
      segment_state_ = SegmentState::kAfterMove;
      last_pt_ = move_to_;
      need_close_ = force_close_;
      break;
    case Verb::kLine:
      p_pts[0] = this->consMoveTo();
      p_pts[1] = src_pts[0];
      last_pt_ = src_pts[0];
      close_line_ = false;
      src_pts += 1;
      break;
    case Verb::kConic:
      conic_weights_ += 1;
      // fall-through
    case Verb::kQuad:
      p_pts[0] = this->consMoveTo();
      std::memcpy(std::addressof(p_pts[1]), src_pts, 2 * sizeof(Point));
      last_pt_ = src_pts[1];
      src_pts += 2;
      break;
    case Verb::kCubic:
      p_pts[0] = this->consMoveTo();
      std::memcpy(std::addressof(p_pts[1]), src_pts, 3 * sizeof(Point));
      last_pt_ = src_pts[2];
      src_pts += 3;
      break;
    case Verb::kClose:
      verb = this->autoClose(p_pts);
      if (verb == Verb::kLine) {
        verbs_--;
      } else {
        need_close_ = false;
        segment_state_ = SegmentState::kEmptyContour;
      }
      last_pt_ = move_to_;
      break;
    case Verb::kDone:
      break;
  }
  pts_ = src_pts;
  return verb;
}

float Path::Iter::conicWeight() const { return *conic_weights_; }

bool Path::Iter::isClosedContour() const {
  if (verbs_ == nullptr || verbs_ == verb_stop_) {
    return false;
  }

  if (force_close_) {
    return true;
  }

  auto p_verbs = verbs_;
  auto p_stop = verb_stop_;

  if (*p_verbs == Verb::kMove) {
    p_verbs += 1;
  }

  while (p_verbs < p_stop) {
    auto v = *p_verbs++;
    if (v == Verb::kMove) {
      break;
    }
    if (v == Verb::kClose) {
      return true;
    }
  }

  return false;
}

Path::Verb Path::Iter::autoClose(Point* pts) {
  if (last_pt_ != move_to_) {
    if (FloatIsNan(last_pt_.x) || FloatIsNan(last_pt_.y) ||
        FloatIsNan(move_to_.x) || FloatIsNan(move_to_.y)) {
      return Verb::kClose;
    }

    pts[0] = last_pt_;
    pts[1] = move_to_;
    last_pt_ = move_to_;
    close_line_ = true;
    return Verb::kLine;
  } else {
    pts[0] = move_to_;
    return Verb::kClose;
  }
}

const Point& Path::Iter::consMoveTo() {
  if (segment_state_ == SegmentState::kAfterMove) {
    segment_state_ = SegmentState::kAfterPrimitive;
    return move_to_;
  }

  return pts_[-1];
}

bool Path::Iter::isCloseLine() const { return close_line_; }

Path::RawIter::RawIter()
    : pts_(nullptr),
      verbs_(nullptr),
      verb_stop_(nullptr),
      conic_weights_(nullptr) {}

void Path::RawIter::setPath(const Path& path) {
  pts_ = path.points_.data();
  if (path.countVerbs() > 0) {
    verbs_ = path.verbs_.data();
    verb_stop_ = path.verbs_.data() + path.countVerbs();
  } else {
    verbs_ = verb_stop_ = nullptr;
  }

  conic_weights_ = path.conic_weights_.data();
  if (conic_weights_) {
    conic_weights_ -= 1;
  }

  if (!path.isFinite()) {
    verb_stop_ = verbs_;
  }
}

Path::Verb Path::RawIter::next(Point pts[4]) {
  if (verbs_ == verb_stop_) {
    return Verb::kDone;
  }

  auto verb = *verbs_++;
  auto src_pts = pts_;
  switch (verb) {
    case Verb::kMove:
      pts[0] = src_pts[0];
      src_pts += 1;
      break;

    case Verb::kLine:
      pts[0] = src_pts[-1];
      pts[1] = src_pts[0];
      src_pts += 1;
      break;

    case Verb::kConic:
      conic_weights_ += 1;
      // fall-through
    case Verb::kQuad:
      pts[0] = src_pts[-1];
      pts[1] = src_pts[0];
      pts[2] = src_pts[1];
      src_pts += 2;
      break;
    case Verb::kCubic:
      pts[0] = src_pts[-1];
      pts[1] = src_pts[0];
      pts[2] = src_pts[1];
      pts[3] = src_pts[2];
      src_pts += 3;
      break;
    case Verb::kClose:
    case Verb::kDone:
    default:
      break;
  }
  pts_ = src_pts;
  return verb;
}

Path::Verb Path::RawIter::peek() const {
  return verbs_ < verb_stop_ ? *verbs_ : Verb::kDone;
}

float Path::RawIter::conicWeight() const { return *conic_weights_; }

static int _pts_in_verb(Path::Verb verb) {
  switch (verb) {
    case Path::Verb::kMove:
    case Path::Verb::kLine:
      return 1;
    case Path::Verb::kQuad:
    case Path::Verb::kConic:
      return 2;
    case Path::Verb::kCubic:
      return 3;
    case Path::Verb::kClose:
    case Path::Verb::kDone:
      return 0;
  }
  return 0;
}

Path& Path::moveTo(float x, float y) {
  last_move_to_index_ = countPoints();

  verbs_.emplace_back(Verb::kMove);
  points_.emplace_back(Point{x, y, 0, 1});

  return *this;
}

Path& Path::lineTo(float x, float y) {
  injectMoveToIfNeed();

  verbs_.emplace_back(Verb::kLine);
  points_.emplace_back(Point{x, y, 0, 1});

  return *this;
}

Path& Path::quadTo(float x1, float y1, float x2, float y2) {
  injectMoveToIfNeed();

  verbs_.emplace_back(Verb::kQuad);
  points_.emplace_back(Point{x1, y1, 0, 1});
  points_.emplace_back(Point{x2, y2, 0, 1});
  return *this;
}

Path& Path::conicTo(float x1, float y1, float x2, float y2, float weight) {
  if (!(weight > 0)) {
    this->lineTo(x2, y2);
  } else if (glm::isinf(weight)) {
    this->lineTo(x1, y1);
    this->lineTo(x2, y2);
  } else if (weight == Float1) {
    this->quadTo(x1, y1, x2, y2);
  } else {
    injectMoveToIfNeed();

    verbs_.emplace_back(Verb::kConic);
    conic_weights_.emplace_back(weight);
    points_.emplace_back(Point{x1, y1, 0, 1});
    points_.emplace_back(Point{x2, y2, 0, 1});
  }
  return *this;
}

Path& Path::cubicTo(float x1, float y1, float x2, float y2, float x3,
                    float y3) {
  injectMoveToIfNeed();

  verbs_.emplace_back(Verb::kCubic);

  points_.emplace_back(Point{x1, y1, 0, 1});
  points_.emplace_back(Point{x2, y2, 0, 1});
  points_.emplace_back(Point{x3, y3, 0, 1});

  return *this;
}

Path& Path::arcTo(float x1, float y1, float x2, float y2, float radius) {
  if (radius == 0) {
    return lineTo(x1, y1);
  }

  Point start;
  getLastPt(std::addressof(start));
  // need to know prev point so we can construct tangent vectors
  glm::dvec4 befored, afterd;

  befored = glm::normalize(glm::dvec4{x1 - start.x, y1 - start.y, 0, 0});
  afterd = glm::normalize(glm::dvec4{x2 - x1, y2 - y1, 0, 0});

  double cosh = glm::dot(befored, afterd);
  double sinh = befored.x * afterd.y - befored.y * afterd.x;

  if (!PointIsFinite(befored) || !PointIsFinite(afterd) ||
      FloatNearlyZero(static_cast<float>(sinh))) {
    return lineTo(x1, y1);
  }

  glm::vec4 before = befored;
  glm::vec4 after = afterd;

  float dist = glm::abs(static_cast<float>(radius * (1 - cosh) / sinh));
  float xx = x1 - dist * before.x;
  float yy = y1 - dist * before.y;
  PointSetLength<false>(after, after.x, after.y, dist);

  lineTo(xx, yy);
  float weight = glm::sqrt(static_cast<float>(FloatHalf + cosh * 0.5));

  return conicTo(x1, y1, x1 + after.x, y1 + after.y, weight);
}

Path& Path::arcTo(float rx, float ry, float xAxisRotate, ArcSize largeArc,
                  Direction sweep, float x, float y) {
  this->injectMoveToIfNeed();
  std::array<Point, 2> src_pts{};
  this->getLastPt(src_pts.data());
  // If rx = 0 or ry = 0 then this arc is treated as a straight line segment
  // joining the endpoints.
  // http://www.w3.org/TR/SVG/implnote.html#ArcOutOfRangeParameters
  if (!rx || !ry) {
    return this->lineTo(x, y);
  }
  // If the current point and target point for the arc are identical, it should
  // be treated as a zero length path. This ensures continuity in animations.
  src_pts[1].x = x;
  src_pts[1].y = y;
  src_pts[1].z = 0;
  src_pts[1].w = 1;

  if (src_pts[0] == src_pts[1]) {
    return this->lineTo(x, y);
  }

  rx = std::abs(rx);
  ry = std::abs(ry);
  Vector mid_point_distance = (src_pts[0] - src_pts[1]) * 0.5f;

  Matrix point_transform =
      glm::rotate(glm::identity<Matrix>(), -xAxisRotate, glm::vec3(0, 0, 1.f));

  Point transformed_mid_point = point_transform * mid_point_distance;
  float square_rx = rx * rx;
  float square_ry = ry * ry;
  float square_x = transformed_mid_point.x * transformed_mid_point.x;
  float square_y = transformed_mid_point.y * transformed_mid_point.y;

  // Check if the radii are big enough to draw the arc, scale radii if not.
  //  http://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
  float radii_scale = square_x / square_rx + square_y / square_ry;
  if (radii_scale > 1.f) {
    radii_scale = std::sqrt(radii_scale);
    rx *= radii_scale;
    ry *= radii_scale;
  }

  point_transform =
      glm::scale(glm::identity<Matrix>(), glm::vec3(1.f / rx, 1.f / ry, 1.f));

  point_transform =
      point_transform * glm::rotate(glm::identity<Matrix>(), -xAxisRotate,
                                    glm::vec3{0.f, 0.f, 1.f});

  std::array<Point, 2> unit_pts{};
  unit_pts[0] = point_transform * src_pts[0];
  unit_pts[1] = point_transform * src_pts[1];

  Vector delta = unit_pts[1] - unit_pts[0];
  float d = delta.x * delta.x + delta.y * delta.y;
  float scale_factor_squared = std::max(1.f / d - 0.25f, 0.f);
  float scale_factor = std::sqrt(scale_factor_squared);

  if ((sweep == Direction::kCCW) != (bool)largeArc) {
    scale_factor = -scale_factor;
  }

  PointScale(delta, scale_factor, &delta);
  Point center_point = (unit_pts[0] + unit_pts[1]) * 0.5f;
  center_point.x -= delta.y;
  center_point.y += delta.x;
  unit_pts[0] -= center_point;
  unit_pts[1] -= center_point;
  float theta1 = std::atan2(unit_pts[0].y, unit_pts[0].x);
  float theta2 = std::atan2(unit_pts[1].y, unit_pts[1].x);
  float theta_arc = theta2 - theta1;
  if (theta_arc < 0 && (sweep == Direction::kCW)) {
    // sweep flipped from the original implementation
    theta_arc += glm::pi<float>() * 2.f;
  } else if (theta_arc > 0 && (sweep != Direction::kCW)) {
    theta_arc -= glm::pi<float>() * 2.f;
  }

  // Very tiny angles cause our subsequent math to go wonky (skbug.com/9272)
  // so we do a quick check here. The precise tolerance amount is just made up.
  // PI/million happens to fix the bug in 9272, but a larger value is probably
  // ok too.
  if (std::abs(theta_arc) < (glm::pi<float>() / (1000.f * 1000.f))) {
    return this->lineTo(x, y);
  }

  point_transform = glm::rotate(glm::identity<Matrix>(), xAxisRotate,
                                glm::vec3{0.f, 0.f, 1.f});
  point_transform = point_transform *
                    glm::scale(glm::identity<Matrix>(), glm::vec3{rx, ry, 1.f});

  // the arc may be slightly bigger than 1/4 circle, so allow up to 1/3rd
  int segments =
      std::ceil(std::abs(theta_arc / (2.f * glm::pi<float>() / 3.f)));
  float theta_width = theta_arc / segments;
  float t = std::tan(theta_width * 0.5f);

  if (!FloatIsFinite(t)) {
    return *this;
  }

  float start_theta = theta1;
  float w = std::sqrt(FloatHalf + std::cos(theta_width) * FloatHalf);
  auto float_is_integer = [](float scalar) -> bool {
    return scalar == std::floor(scalar);
  };

  bool expect_integers =
      FloatNearlyZero(glm::pi<float>() / 2.f - std::abs(theta_width)) &&
      float_is_integer(rx) && float_is_integer(ry) && float_is_integer(x) &&
      float_is_integer(y);

  for (int i = 0; i < segments; i++) {
    float end_theta = start_theta + theta_width;
    float sin_end_theta = FloatSinSnapToZero(end_theta);
    float cos_end_theta = FloatCosSnapToZero(end_theta);

    PointSet(unit_pts[1], cos_end_theta, sin_end_theta);
    unit_pts[1] += center_point;
    unit_pts[1].w = 1;  // FIXME

    unit_pts[0] = unit_pts[1];
    unit_pts[0].x += t * sin_end_theta;
    unit_pts[0].y += -t * cos_end_theta;

    std::array<Point, 2> mapped;
    mapped[0] = unit_pts[0] * point_transform;
    mapped[1] = unit_pts[1] * point_transform;

    if (expect_integers) {
      for (auto& point : mapped) {
        point.x = std::round(point.x);
        point.y = std::round(point.y);
      }
    }

    this->conicTo(mapped[0], mapped[1], w);
    start_theta = end_theta;
  }

  return *this;
}

Path& Path::addRect(Rect const& rect, Direction dir, uint32_t start) {
  this->setFirstDirection(this->hasOnlyMoveTos() ? dir : Direction::kUnknown);

  AutoDisableDirectionCheck addc{this};
  AutoPathBoundsUpdate adbu(this, rect);

  Path_RectPointIterator iter{rect, dir, start};

  this->moveTo(iter.current());
  this->lineTo(iter.next());
  this->lineTo(iter.next());
  this->lineTo(iter.next());
  this->close();

  return *this;
}

Path& Path::close() {
  size_t count = countVerbs();
  if (count > 0) {
    switch (verbs_.back()) {
      case Verb::kLine:
      case Verb::kQuad:
      case Verb::kConic:
      case Verb::kCubic:
      case Verb::kMove:
        verbs_.emplace_back(Verb::kClose);
        break;
      case Verb::kClose:
        break;
      default:
        break;
    }
  }

  last_move_to_index_ ^=
      ~last_move_to_index_ >> (8 * sizeof(last_move_to_index_) - 1);

  return *this;
}

Path& Path::addRoundRect(Rect const& rect, float rx, float ry, Direction dir) {
  if (rx < 0 || ry < 0) {
    return *this;
  }

  RRect rrect;
  rrect.setRectXY(rect, rx, ry);

  return this->addRRect(rrect, dir);
}

Path& Path::addRRect(RRect const& rrect, Direction dir) {
  // legacy start indices: 6 (CW) and 7(CCW)
  return this->addRRect(rrect, dir, dir == Direction::kCW ? 6 : 7);
}

Path& Path::addRRect(RRect const& rrect, Direction dir, uint32_t start_index) {
  bool is_rrect = hasOnlyMoveTos();

  Rect const& bounds = rrect.getBounds();

  if (rrect.isRect() || rrect.isEmpty()) {
    // degenerate(rect) => radii points are collapsing
    this->addRect(bounds, dir, (start_index + 1) / 2);
  } else if (rrect.isOval()) {
    // degenerate(oval) => line points are collapsing
    this->addOval(bounds, dir, start_index / 2);
  } else {
    this->setFirstDirection(this->hasOnlyMoveTos() ? dir : Direction::kUnknown);

    AutoPathBoundsUpdate apbu{this, bounds};
    AutoDisableDirectionCheck addc{this};

    // we start with a conic on odd indices when moving CW vs.
    // even indices when moving CCW
    bool starts_with_conic = ((start_index & 1) == (dir == Direction::kCW));
    float weight = FloatRoot2Over2;

    int initial_verb_count = this->countVerbs();
    int verbs = starts_with_conic
                    ? 9    // moveTo + 4x conicTo + 3x lineTo + close
                    : 10;  // moveTo + 4x lineTo + 4x conicTo + close

    Path_RRectPointIterator rrect_iter{rrect, dir, start_index};
    // Corner iterator indices follow the collapsed radii model,
    // adjusted such that the start pt is "behind" the radii start pt.
    uint32_t rect_start_index =
        start_index / 2 + (dir == Direction::kCW ? 0 : 1);
    Path_RectPointIterator rect_iter{bounds, dir, rect_start_index};

    this->moveTo(rrect_iter.current());
    if (starts_with_conic) {
      for (uint32_t i = 0; i < 3; i++) {
        this->conicTo(rect_iter.next(), rrect_iter.next(), weight);
        this->lineTo(rrect_iter.next());
      }
      this->conicTo(rect_iter.next(), rrect_iter.next(), weight);
      // final lineTo handled by close().
    } else {
      for (uint32_t i = 0; i < 4; i++) {
        this->lineTo(rrect_iter.next());
        this->conicTo(rect_iter.next(), rrect_iter.next(), weight);
      }
    }

    this->close();
  }

  return *this;
}

Path& Path::reset() {
  *this = Path();
  return *this;
}

Path& Path::reverseAddPath(const Path& src) {
  auto verbs_begin = src.verbs_.data();
  auto verbs = verbs_begin + src.verbs_.size();
  auto pts = src.points_.data() + src.countPoints();
  auto conic_weights = src.conic_weights_.data() + src.conic_weights_.size();

  bool need_move = true;
  bool need_close = false;
  while (verbs > verbs_begin) {
    auto v = *--verbs;
    int n = _pts_in_verb(v);

    if (need_move) {
      --pts;
      moveTo(pts->x, pts->y);
      need_move = false;
    }

    pts -= n;

    switch (v) {
      case Verb::kMove:
        if (need_close) {
          close();
          need_close = false;
        }
        need_move = true;
        pts += 1;
        break;
      case Verb::kLine:
        lineTo(pts->x, pts->y);
        break;
      case Verb::kQuad:
        quadTo(pts[1].x, pts[1].y, pts[0].x, pts[0].y);
        break;
      case Verb::kConic:
        conicTo(pts[1], pts[0], *--conic_weights);
        break;
      case Verb::kCubic:
        cubicTo(pts[2].x, pts[2].y, pts[1].x, pts[1].y, pts[0].x, pts[0].y);
        break;
      case Verb::kClose:
        need_close = true;
        break;
      default:
        break;
    }
  }

  return *this;
}

Path& Path::addCircle(float x, float y, float radius, Direction dir) {
  if (radius > 0) {
    addOval(Rect::MakeLTRB(x - radius, y - radius, x + radius, y + radius),
            dir);
  }

  return *this;
}

Path& Path::addOval(const Rect& oval, Direction dir) {
  return addOval(oval, dir, 1);
}

Path& Path::addOval(const Rect& oval, Direction dir, uint32_t start) {
  bool is_oval = hasOnlyMoveTos();
  if (is_oval) {
    first_direction_ = dir;
  } else {
    first_direction_ = Direction::kUnknown;
  }

  AutoDisableDirectionCheck addc{this};
  AutoPathBoundsUpdate apbu{this, oval};

  int kVerbs = 6;  // moveTo + 4x conicTo + close

  Path_OvalPointIterator oval_iter{oval, dir, start};
  Path_RectPointIterator rect_iter{oval, dir,
                                   start + (dir == Direction::kCW ? 0 : 1)};

  float weight = FloatRoot2Over2;

  moveTo(oval_iter.current());
  for (uint32_t i = 0; i < 4; i++) {
    conicTo(rect_iter.next(), oval_iter.next(), weight);
  }
  close();
  return *this;
}

Path& Path::reversePathTo(const Path& src) {
  if (src.verbs_.empty()) {
    return *this;
  }

  auto verbs = src.verbs_.data() + src.verbs_.size();
  auto verbs_begin = src.verbs_.data();
  const Point* pts = src.points_.data() + src.points_.size() - 1;
  const float* conic_weights =
      src.conic_weights_.data() + src.conic_weights_.size();

  while (verbs > verbs_begin) {
    auto v = *--verbs;
    pts -= _pts_in_verb(v);
    switch (v) {
      case Verb::kMove:
        // if the path has multiple contours, stop after reversing the last
        return *this;
      case Verb::kLine:
        lineTo(pts[0].x, pts[0].y);
        break;
      case Verb::kQuad:
        quadTo(pts[1].x, pts[1].y, pts[0].x, pts[0].y);
        break;
      case Verb::kConic:
        conicTo(pts[1].x, pts[1].y, pts[0].x, pts[0].y, *--conic_weights);
        break;
      case Verb::kCubic:
        cubicTo(pts[2].x, pts[2].y, pts[1].x, pts[1].y, pts[0].x, pts[0].y);
        break;
      case Verb::kClose:
      case Verb::kDone:
      default:
        break;
    }
  }

  return *this;
}

bool Path::getLastPt(Point* lastPt) const {
  size_t count = countPoints();
  if (count > 0) {
    if (lastPt) {
      *lastPt = points_.back();
    }
    return true;
  }

  if (lastPt) {
    *lastPt = {0, 0, 0, 1};
  }
  return false;
}

Point Path::getPoint(int index) const {
  if (index < countPoints()) {
    return points_[index];
  }
  return Point{0, 0, 0, 1};
}

bool Path::isFinite() const {
  computeBounds();
  return is_finite_;
}

bool Path::isLine(Point* line) const {
  int verb_count = this->countVerbs();

  if (2 == verb_count) {
    assert(verbs_.front() == Verb::kMove);
    if (verbs_[1] == Verb::kLine) {
      assert(2 == this->countPoints());
      if (line) {
        const Point* pts = points();
        line[0] = pts[0];
        line[1] = pts[1];
      }
    }
  }

  return false;
}

bool Path::operator==(const Path& other) {
  return (this == std::addressof(other)) ||
         (last_move_to_index_ == other.last_move_to_index_ &&
          convexity_ == other.convexity_ && is_finite_ == other.is_finite_ &&
          points_.data() == other.points_.data() &&
          verbs_.data() == other.verbs_.data() &&
          conic_weights_.data() == other.conic_weights_.data());
}

void Path::swap(Path& that) {
  if (this != &that) {
    std::swap(last_move_to_index_, that.last_move_to_index_);
    std::swap(convexity_, that.convexity_);
    std::swap(points_, that.points_);
    std::swap(verbs_, that.verbs_);
    std::swap(conic_weights_, that.conic_weights_);
    std::swap(is_finite_, that.is_finite_);
  }
}

Path& Path::addPath(const Path& src, float dx, float dy, AddMode mode) {
  Matrix matrix = glm::translate(glm::identity<Matrix>(), glm::vec3{dx, dy, 0});
  return addPath(src, matrix, mode);
}

Path& Path::addPath(const Path& src, AddMode mode) {
  return addPath(src, glm::identity<Matrix>(), mode);
}

Path& Path::addPath(const Path& src, const Matrix& matrix, AddMode mode) {
  if (src.isEmpty()) {
    return *this;
  }
  // Detect if we're trying to add ourself
  auto p_src = std::addressof(src);
  Path* tmp = nullptr;
  if (this == p_src) {
    tmp = this;
  }

  if (mode == AddMode::kAppend) {
    if (src.last_move_to_index_ >= 0) {
      last_move_to_index_ = countPoints() + src.last_move_to_index_;
    }

    // add verb
    verbs_.insert(verbs_.end(), src.verbs_.begin(), src.verbs_.end());
    // add weights
    conic_weights_.insert(conic_weights_.end(), src.conic_weights_.begin(),
                          src.conic_weights_.end());
    // add points
    //    points_.insert(points_.end(), src.points_.begin(), src.points_.end());
    for (const auto& p : src.points_) {
      points_.emplace_back(p * matrix);
    }

    return *this;
  }

  RawIter iter{*this};
  Point pts[4];
  Verb verb;
  bool first_verb = true;

  while ((verb = iter.next(pts)) != Verb::kDone) {
    switch (verb) {
      case Verb::kMove:
        pts[0] = pts[0] * matrix;
        if (first_verb && !isEmpty()) {
          injectMoveToIfNeed();
          Point last_pt;
          if (last_move_to_index_ < 0 || !getLastPt(std::addressof(last_pt)) ||
              last_pt != pts[0]) {
            lineTo(pts[0].x, pts[0].y);
          }
        } else {
          moveTo(pts[0].x, pts[0].y);
        }
        break;
      case Verb::kLine:
        pts[1] = pts[1] * matrix;
        lineTo(pts[1].x, pts[1].y);
        break;
      case Verb::kQuad:
        pts[1] = pts[1] * matrix;
        pts[2] = pts[2] * matrix;
        quadTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y);
        break;
      case Verb::kConic:
        pts[1] = pts[1] * matrix;
        pts[2] = pts[2] * matrix;
        conicTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y, iter.conicWeight());
        break;
      case Verb::kCubic:
        pts[1] = pts[1] * matrix;
        pts[2] = pts[2] * matrix;
        pts[3] = pts[3] * matrix;
        cubicTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y, pts[3].x, pts[3].y);
        break;
      case Verb::kClose:
        close();
        break;
      default:
        break;
    }
    first_verb = false;
  }
  return *this;
}

void Path::setLastPt(float x, float y) {
  if (countPoints() == 0) {
    moveTo(x, y);
  } else {
    PointSet(points_.back(), x, y);
  }
}

Path Path::copyWithMatrix(const Matrix& matrix) const {
  Path ret;

  ret.last_move_to_index_ = last_move_to_index_;
  ret.convexity_ = convexity_;
  ret.first_direction_ = first_direction_;

  for (const auto& p : this->points_) {
    ret.points_.emplace_back(matrix * p);
  }

  ret.conic_weights_ = conic_weights_;
  ret.verbs_ = verbs_;

  ret.is_finite_ = is_finite_;
  ret.bounds_ = bounds_;

  return ret;
}

Path Path::copyWithScale(float scale) const {
  Path ret;

  ret.last_move_to_index_ = last_move_to_index_;
  ret.convexity_ = convexity_;
  ret.first_direction_ = first_direction_;

  ret.points_.reserve(this->points_.size());
  for (auto p : this->points_) {
    p.x *= scale;
    p.y *= scale;
    ret.points_.emplace_back(p);
  }

  ret.conic_weights_.resize(conic_weights_.size());
  std::memcpy(ret.conic_weights_.data(), conic_weights_.data(),
              conic_weights_.size() * sizeof(float));
  ret.verbs_.resize(verbs_.size());
  std::memcpy(ret.verbs_.data(), verbs_.data(), verbs_.size() * sizeof(Verb));

  ret.is_finite_ = is_finite_;
  ret.bounds_ = bounds_;

  return ret;
}

void Path::injectMoveToIfNeed() {
  if (last_move_to_index_ < 0) {
    float x, y;
    if (countVerbs() == 0) {
      x = y = 0;
    } else {
      Point const& pt = atPoint(~last_move_to_index_);
      x = pt.x;
      y = pt.y;
    }
    moveTo(x, y);
  }
}

void Path::computeBounds() const {
  is_finite_ = ComputePtBounds(std::addressof(bounds_), *this);
}

bool Path::hasOnlyMoveTos() const {
  for (auto it : verbs_) {
    if (it == Verb::kLine || it == Verb::kQuad || it == Verb::kConic ||
        it == Verb::kCubic) {
      return false;
    }
  }
  return true;
}

bool Path::ComputePtBounds(Rect* bounds, const Path& ref) {
  return bounds->setBoundsCheck(ref.points_.data(), ref.countPoints());
}

bool Path::isZeroLengthSincePoint(int startPtIndex) const {
  int32_t count = countPoints() - startPtIndex;
  if (count < 2) {
    return true;
  }

  auto pts = points_.data() + startPtIndex;
  Point const& first = *pts;

  for (int32_t index = 1; index < count; index++) {
    if (first != pts[index]) {
      return false;
    }
  }
  return true;
}

static void append_params(std::ostream& os, const std::string& label,
                          const Point pts[], int count,
                          float conicWeight = -12345) {
  os << label << "(";
  for (int i = 0; i < count; i++) {
    const Point* point = pts + i;
    float x = point->x;
    float y = point->y;
    std::stringstream ss;
    ss << "( " << x << ", " << y << ")";
    os << ss.str();
  }
  if (conicWeight != -12345) {
    os << ", " << conicWeight;
  }
  os << ");";
  os << std::endl;
}

void Path::dump() {
  Iter iter{*this, false};

  Point pts[4];
  Verb verb;

  std::ostringstream res;

  while ((verb = iter.next(pts)) != Verb::kDone) {
    switch (verb) {
      case Verb::kMove:
        append_params(res, "path.moveTo", std::addressof(pts[0]), 1);
        break;
      case Verb::kLine:
        append_params(res, "path.lineTo", std::addressof(pts[1]), 1);
        break;
      case Verb::kQuad:
        append_params(res, "path.quadTo", std::addressof(pts[1]), 2);
        break;
      case Verb::kConic:
        append_params(res, "path.conicTo", std::addressof(pts[1]), 2,
                      iter.conicWeight());
        break;
      case Verb::kCubic:
        append_params(res, "path.cubicTo", std::addressof(pts[1]), 3);
        break;
      case Verb::kClose:
        append_params(res, "path.close()", nullptr, 0);
        break;
      default:
        verb = Verb::kDone;
        break;
    }
  }
  LOG_INFO("------ {}", res.str());
}

}  // namespace skity
