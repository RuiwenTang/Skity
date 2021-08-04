#ifndef SKITY_SRC_GRAPHIC_PATH_MEASURE_HPP
#define SKITY_SRC_GRAPHIC_PATH_MEASURE_HPP

#include <skity/graphic/path.hpp>

#include "src/geometry/contour_measure.hpp"

namespace skity {

/**
 * @class PathMeasure
 *	Util class to measure path length
 */
class PathMeasure final {
 public:
  PathMeasure();
  PathMeasure(Path const& path, bool forceClosed, float resScale = 1.f);

  ~PathMeasure();

  void setPath(Path const* path, bool forceClosed);

  /**
   * Return the total length of the current contour, or 0 if no path is
   * associated.
   *
   * @return total length of current contour
   */
  float getLength();

  /**
   * @brief Get the Pos Tan object
   *
   * @param distance
   * @param position
   * @param tangent
   * @return true
   * @return false
   */
  bool getPosTan(float distance, Point* position, Vector* tangent);

  bool getSegment(float startD, float stopD, Path* dst, bool startWithMoveTo);

  bool isClosed();

  bool nextContour();

 private:
  ContourMeasureIter iter_;
  std::shared_ptr<ContourMeasure> contour_;
};

}  // namespace skity

#endif  // SKITY_SRC_GRAPHIC_PATH_MEASURE_HPP