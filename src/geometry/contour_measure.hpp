#ifndef SKITY_SRC_GEOMETRY_CONTOUR_MEASURE_HPP
#define SKITY_SRC_GEOMETRY_CONTOUR_MEASURE_HPP

#include <memory>
#include <skity/geometry/point.hpp>
#include <skity/graphic/path.hpp>
#include <vector>

namespace skity {

struct Conic;

class ContourMeasure {
 public:
  enum SegType {
    kLine_SegType,
    kQuad_SegType,
    kCubic_SegType,
    kConic_SegType,
  };
  ~ContourMeasure() = default;

  float length() const { return length_; }

  bool getPosTan(float distance, Point* position, Vector* tangent) const;

  bool getSegment(float startD, float stopD, Path* dst,
                  bool startWithMoveTo) const;

  bool isClosed() const { return is_closed_; }

  struct Segment {
    // total distance up to this point
    float distance;
    // index into the pts array
    uint32_t pt_index;
    uint32_t t_value : 30;
    uint32_t type : 2;  // actually the enum SegType

    float getScalarT() const;

    static const Segment* Next(Segment const* seg) {
      uint32_t pt_index = seg->pt_index;
      do {
        seg++;
      } while (seg->pt_index == pt_index);
      return seg;
    }
  };

  ContourMeasure(std::vector<Segment>&& segs, std::vector<Point>&& pts,
                 float length, bool isClosed);

 private:
  const Segment* distanceToSegment(float distance, float* t) const;

  friend class ContourMeasureiter;

 private:
  std::vector<Segment> segments_;
  // Points used to define the segments
  std::vector<Point> pts_;
  float length_;
  bool is_closed_;
};

class ContourMeasureIter {
 public:
  ContourMeasureIter();
  /**
   * @brief Construct a new Contour Measure Iter object with path.
   *
   * @param path
   * @param forceClosed
   * @param resScale controls the precision of the measure. values > 1 increase
   * the precision (and possibly slow down the computation).
   */
  ContourMeasureIter(Path const& path, bool forceClosed, float resScale = 1.f);

  ~ContourMeasureIter();

  void reset(Path const&, bool forceClosed, float resScale = 1.f);

  std::shared_ptr<ContourMeasure> next();

  class Impl;

  std::unique_ptr<Impl> impl_;
};

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_CONTOUR_MEASURE_HPP