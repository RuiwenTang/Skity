
#include "src/render/path_vertex.hpp"

#include "src/geometry/math.hpp"

namespace skity {

size_t PathVertex::appendPoint(const Point& point)
{
  size_t index = point_buffer.size();
  point_buffer.emplace_back(point);
  return index;
}

void PathVertex::frontTriangle(uint32_t p1, uint32_t p2, uint32_t p3)
{
  front_vertices.emplace_back(p1);
  front_vertices.emplace_back(p2);
  front_vertices.emplace_back(p3);
}

void PathVertex::backTriangle(uint32_t p1, uint32_t p2, uint32_t p3)
{
  back_vertices.emplace_back(p1);
  back_vertices.emplace_back(p2);
  back_vertices.emplace_back(p3);
}

size_t PathVertexBuilder::appendPoint(const Point& point)
{
  return path_vertex->appendPoint(point);
}

void PathVertexBuilder::appendTriangle(uint32_t p1, uint32_t p2, uint32_t p3)
{
  if (p1 == p2 || p2 == p3) {
    return;
  }
  Point const& point1 = path_vertex->getPoint(p1);
  Point const& point2 = path_vertex->getPoint(p2);
  Point const& point3 = path_vertex->getPoint(p3);

  Orientation orientation = CalculateOrientation(point1, point2, point3);
  if (orientation == Orientation::kClockWise) {
    path_vertex->frontTriangle(p1, p2, p3);
  }
  else {
    path_vertex->backTriangle(p1, p2, p3);
  }
}

}  // namespace skity
