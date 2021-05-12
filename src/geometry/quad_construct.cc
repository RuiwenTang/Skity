#include "src/geometry/quad_construct.hpp"

#include "src/geometry/math.hpp"

namespace skity {

bool QuadConstruct::init(float start, float end) {
  this->startT = start;
  this->midT = (start + end) * FloatHalf;
  this->endT = end;
  startSet = endSet = false;

  return startT < midT && midT < endT;
}

bool QuadConstruct::initWithStart(QuadConstruct* parent) {
  if (!init(parent->startT, parent->midT)) {
    return false;
  }

  quad[0] = parent->quad[0];
  tangentStart = parent->tangentStart;
  startSet = true;
  return true;
}

bool QuadConstruct::initWithEnd(QuadConstruct* parent) {
  if (!init(parent->midT, parent->endT)) {
    return false;
  }

  quad[2] = parent->quad[2];
  tangentEnd = parent->tangentEnd;
  endSet = true;
  return true;
}

}  // namespace skity
