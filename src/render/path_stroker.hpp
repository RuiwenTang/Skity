#ifndef SKITY_SRC_RENDER_PATH_STROKER_HPP
#define SKITY_SRC_RENDER_PATH_STROKER_HPP

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class PathStroker {
 public:
  enum class StrokeType {
    kOuter,
    kInner,
  };

  enum class ResultType {
    // caller should split the quad stroke in two
    kSplit,
    // caller should add line
    kDegenerate,
    // the caller should (continue to try to) add a quad stroke
    kQuad,
  };

  enum class ReductionType {
    // all curve points are practically identical
    kPoint,
    // the control point is on the line between the ends
    kLine,
    // the control point is outside the line between the ends
    kQuad,
    // the control point is on the line but outside the ends
    kDegenerate,
    kDegenerate2,
    kDegenerate3,
  };

  enum class IntersectRayType {
    kCtrlPt,
    kResult,
  };

  PathStroker(Path const& src, float radius, float miterLimit, Paint::Cap cap,
              Paint::Join join, float resScale, bool canIgnoreCenter);

};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_PATH_STROKER_HPP

