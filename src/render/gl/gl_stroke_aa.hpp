#ifndef SKITY_SRC_RENDER_GL_GL_STROKE_AA_HPP
#define SKITY_SRC_RENDER_GL_GL_STROKE_AA_HPP

#include <skity/geometry/point.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class GLVertex;
struct GLMeshRange;

/**
 * This is only used for anti-alias when do path fill and not handle stroke_join
 *
 */
class GLStrokeAA final {
 public:
  explicit GLStrokeAA(float aa_width);
  ~GLStrokeAA() = default;

  GLMeshRange StrokePathAA(Path const& path, GLVertex* gl_vertex);

 private:
  void HandleLineTo(Point const& from, Point const& to);
  void HandleQuadTo(Point const& start, Point const& control, Point const& end);
  void HandleConicTo(Point const& start, Point const& control, Point const& end,
                     float weight);
  void HandleCubicTo(Point const& start, Point const& control1,
                     Point const& control2, Point const& end);

  void AppendQuadOrSplitRecursively(std::array<Point, 3> const& quad);

 private:
  float aa_width_;
  GLVertex* gl_vertex_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_STROKE_AA_HPP