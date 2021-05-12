
#ifndef SKITY_SRC_GEOMETRY_QUAD_CONSTRUCT_HPP
#define SKITY_SRC_GEOMETRY_QUAD_CONSTRUCT_HPP

#include <skity/geometry/point.hpp>

namespace skity {

/**
 * @sruct QuadConstruct
 * The state of the quad stroke under construction
 */
struct QuadConstruct {
  QuadConstruct() = default;
  ~QuadConstruct() = default;
  // the stroked quad parallel to the original curve
  Point quad[3] = {};
  // a point tangent to quad[0]
  Point tangentStart = {};
  // a point tangent to quad[2]
  Point tangentEnd = {};
  // a segment of the original curve
  float startT = -1;
  float midT = -1;
  float endT = -1;
  // state to share common points across structs
  bool startSet = false;
  bool endSet = false;
  // set if conicident tangents have opposite directions
  bool opposieTangents = false;

  /**
   * init self with properties
   *
   * @param start
   * @param end
   *
   * @return false if start and end are too close to have a unique middle
   */
  bool init(float start, float end);
  bool initWithStart(QuadConstruct* parent);
  bool initWithEnd(QuadConstruct* parent);
};

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_QUAD_CONSTRUCT_HPP
