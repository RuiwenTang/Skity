#ifndef SKITY_SRC_RENDER_GL_GL_FILL_HPP
#define SKITY_SRC_RENDER_GL_GL_FILL_HPP

#include <skity/geometry/point.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class GLVertex;
struct GLMeshRange;

class GLFill final {
 public:
  GLFill() = default;
  ~GLFill() = default;

  GLMeshRange fillPath(Path const& path, Paint const& paint,
                       GLVertex* gl_vertex);

 private:
  void HandleMoveTo(Point const& pt);
  void HandleLineTo(Point const& from, Point const& to);
  void HandleQuadTo(Point const& start, Point const& control, Point const& end);
  void HandleConicTo(Point const& start, Point const& control, Point const& end,
                     float weight);
  void HandleCubicTo(Point const& start, Point const& control1,
                     Point const& control2, Point const& end);
  void HandleClose();

 private:
  GLVertex* gl_vertex_ = nullptr;
  Point start_pt_ = {};
  uint32_t start_pt_index_ = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_FILL_HPP