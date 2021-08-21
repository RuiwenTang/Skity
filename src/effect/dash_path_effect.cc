#include "src/effect/dash_path_effect.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/graphic/path_measure.hpp"

namespace skity {

#define kMaxDashCount 1000000

static inline int is_even(int x) { return !(x & 1); }

static float find_first_interval(const float intervals[], float phase,
                                 int32_t* index, int32_t count) {
  for (int32_t i = 0; i < count; i++) {
    float gap = intervals[i];
    if (phase > gap || (phase == gap && gap)) {
      phase -= gap;
    } else {
      *index = i;
      return gap - phase;
    }
  }
  // If we get here, phase "appears" to be larger than our length. This
  // shouldn't happen with perfect precision, but we can accumulate errors
  // during the initial length computation (rounding can make our sum be too
  // big or too small). In that event, we just have to eat the error here.
  *index = 0;
  return intervals[0];
}

class SpecialLineRec final {
 public:
  SpecialLineRec() = default;
  ~SpecialLineRec() = default;

  bool Init(Path const& src, Path* dst, Paint const& paint,
            int32_t interval_count, float interval_length) {
    if (paint.getStrokeCap() != Paint::kButt_Cap) {
      return false;
    }

    if (!src.isLine(pts_.data())) {
      return false;
    }

    float path_length = PointDistance(pts_[0], pts_[1]);

    tangent_ = pts_[1] - pts_[0];
    if (tangent_.x == 0 && tangent_.y == 0) {
      return false;
    }

    path_length_ = path_length;
    tangent_ *= FloatInvert(path_length);
    PointRotateCCW(tangent_, &normal_);
    normal_ *= SkityFloatHalf(paint.getStrokeWidth());

    float pt_count = path_length * interval_count / (float)interval_length;
    pt_count = std::min(pt_count, (float)kMaxDashCount);

    int n = static_cast<int32_t>(std::ceil(pt_count)) << 2;

    return true;
  }

 private:
  std::array<Point, 2> pts_ = {};
  Vector tangent_ = {};
  Vector normal_ = {};
  float path_length_ = 0.f;
};

DashPathEffect::DashPathEffect(const float* intervals, int32_t count,
                               float phase)
    : PathEffect(),
      phase_(0),
      initial_dash_length_(-1),
      initial_dash_index_(0),
      interval_length_(0) {
  assert(intervals);
  assert(count > 1 && (0 == (count & 1)));

  intervals_.reset(new float[count]);
  count_ = count;
  for (int32_t i = 0; i < count; i++) {
    intervals_[i] = intervals[i];
  }

  this->CalcDashParameters(phase);
}

bool DashPathEffect::onFilterPath(Path* dst, const Path& src, bool stroke,
                                  Paint const& paint) const {
  // do nothing if src wants to be filled
  if (!stroke) {
    return false;
  }

  const float* intervals = this->intervals_.get();
  float dash_count = 0;
  int32_t seg_count = 0;

  Path cull_path_storage;
  const Path* src_ptr = &src;

  PathMeasure meas{*src_ptr, false};

  do {
    bool skip_first_segment = meas.isClosed();
    bool added_segment = false;
    float length = meas.getLength();
    int32_t index = initial_dash_index_;

    // Since the path length / dash length ratio may be arbitrarily large, we
    // can exert significant memory pressure while attempting to build the
    // filtered path. To avoid this, we simply give up dashing beyond a certain
    // threshold.
    dash_count += length * (count_ >> 1) / interval_length_;
    if (dash_count > kMaxDashCount) {
      dst->reset();
      return false;
    }

    // Using double precision to avoid looping indefinitely due to single
    // precision rounding (for extreme path_length/dash_length ratios).
    double distance = 0;
    double dlen = initial_dash_length_;

    while (distance < length) {
      assert(dlen >= 0);
      added_segment = false;
      if (is_even(index) && !skip_first_segment) {
        added_segment = true;
        ++seg_count;

        // TODO: handle special line
        meas.getSegment(static_cast<float>(distance),
                        static_cast<float>(distance + dlen), dst, true);
      }

      distance += dlen;

      // clear this so we only respect it the first time around
      skip_first_segment = false;

      // wrap around our intervals array if necessary
      index += 1;
      assert(index <= count_);
      if (index == count_) {
        index = 0;
      }

      // fetch our next dlen;
      dlen = intervals_[index];
    }

    // extend if we ended on a segment and we need to join up with the (skipped)
    // initial segment
    if (meas.isClosed() && is_even(initial_dash_index_) &&
        initial_dash_length_ >= 0) {
      meas.getSegment(0, initial_dash_length_, dst, !added_segment);
      ++seg_count;
    }
  } while (meas.nextContour());

  return true;
}

PathEffect::DashType DashPathEffect::onAsADash(DashInfo* info) const {
  if (info) {
    if (info->count >= count_ && info->intervals) {
      std::memcpy(info->intervals, intervals_.get(), count_ * sizeof(float));
    }

    info->count = count_;
    info->phase = phase_;
  }

  return PathEffect::DashType::kDash;
}

void DashPathEffect::CalcDashParameters(float phase) {
  float len = 0;
  for (int i = 0; i < count_; i++) {
    len += intervals_[i];
  }
  // update internal_length_
  interval_length_ = len;
  // Adjust phase to be between 0 and len. "flipping" phase if negative.
  // e.g., if len is 100, then phase of -20 (or -120) is equivalent to 80
  if (phase < 0) {
    phase = -phase;
    if (phase > len) {
      phase = std::fmod(phase, len);
    }
    phase = len - phase;

    // Due to finite precision, it's possible that phase == len,
    // even after the subtract (if len >>> phase), so fix that here.
    assert(phase <= len);
    if (phase == len) {
      phase = 0;
    }
  } else if (phase >= len) {
    phase = std::fmod(phase, len);
  }
  // update phase_
  phase_ = phase;

  assert(phase >= 0 && phase < len);

  initial_dash_length_ = find_first_interval(intervals_.get(), phase,
                                             &initial_dash_index_, count_);

  assert(initial_dash_length_ > 0);
  assert(initial_dash_index_ >= 0 && initial_dash_index_ < count_);
}

}  // namespace skity
