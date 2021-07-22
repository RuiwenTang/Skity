
#include <array>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <skity/graphic/path.hpp>
#include <sstream>

#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"

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
  // TODO implement
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
    points_.insert(points_.end(), src.points_.begin(), src.points_.end());

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

  std::cout << "-----------" << std::endl
            << res.str() << "-----------" << std::endl;
}

}  // namespace skity
