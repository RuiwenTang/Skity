#ifndef SKITY_RENDER_PATH_VERTEX_HPP
#define SKITY_RENDER_PATH_VERTEX_HPP

#include <memory>
#include <skity/geometry/point.hpp>
#include <vector>

namespace skity {

class PathVertexBuilder;
/**
 * FrontVertices, stencil buffer ++
 * backVertices, stencil buffer --
 */
class PathVertex {
 public:
  ~PathVertex() = default;

  std::vector<uint32_t> const& frontVerticesIndex() const {
    return front_vertices;
  }
  std::vector<uint32_t> const& backVerticesIndex() const {
    return back_vertices;
  }

  std::vector<Point> const& vertices() const { return point_buffer; }

  void dump();

 private:
  PathVertex() = default;
  friend class PathVertexBuilder;

  size_t appendPoint(Point const& point);

  void frontTriangle(uint32_t p1, uint32_t p2, uint32_t p3);
  void backTriangle(uint32_t p1, uint32_t p2, uint32_t p3);

  Point const& getPoint(size_t index) {
    if (index < point_buffer.size()) {
      return point_buffer[index];
    } else {
      static Point empty{0, 0, 0, 0};
      return empty;
    }
  }

 private:
  std::vector<Point> point_buffer;
  std::vector<uint32_t> front_vertices;
  std::vector<uint32_t> back_vertices;
};

class PathVertexBuilder final {
 public:
  PathVertexBuilder() : path_vertex() { path_vertex.reset(new PathVertex); }
  ~PathVertexBuilder() = default;

  std::unique_ptr<PathVertex> build() { return std::move(path_vertex); }

  size_t appendPoint(Point const& point);

  void appendTriangle(uint32_t p1, uint32_t p2, uint32_t p3);

 private:
  std::unique_ptr<PathVertex> path_vertex;
};

}  // namespace skity

#endif  // SKITY_RENDER_PATH_VERTEX_HPP
