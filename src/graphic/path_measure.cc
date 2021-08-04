#include "src/graphic/path_measure.hpp"

namespace skity {
PathMeasure::PathMeasure() = default;

PathMeasure::PathMeasure(Path const& path, bool forceClosed, float resScale)
    : iter_(path, forceClosed, resScale) {
  contour_ = iter_.next();
}

PathMeasure::~PathMeasure() = default;

void PathMeasure::setPath(const Path* path, bool forceClosed) {
  iter_.reset(path ? *path : Path{}, forceClosed);
  contour_ = iter_.next();
}

float PathMeasure::getLength() {
  if (contour_) {
    return contour_->length();
  } else {
    return 0;
  }
}

bool PathMeasure::getPosTan(float distance, Point* position, Vector* tangent) {
  return contour_ && contour_->getPosTan(distance, position, tangent);
}

bool PathMeasure::getSegment(float startD, float stopD, Path* dst,
                             bool startWithMoveTo) {
  return contour_ && contour_->getSegment(startD, stopD, dst, startWithMoveTo);
}

bool PathMeasure::isClosed() { return contour_ && contour_->isClosed(); }

bool PathMeasure::nextContour() {
  contour_ = iter_.next();
  return !!contour_;
}

}  // namespace skity