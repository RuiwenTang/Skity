#ifndef SKITY_SRC_RENDER_GL_GL_STROKE_HPP
#define SKITY_SRC_RENDER_GL_GL_STROKE_HPP

#include <skity/geometry/point.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class GLVertex;
struct GLMeshRange;

/**
 * @class GLStroke
 *  Used to generate Mesh for path stroke
 *  @todo generate vertex during stroke, not at the end
 */
class GLStroke {
 public:
  explicit GLStroke(Paint const& paint);
  ~GLStroke() = default;

  GLMeshRange strokePath(Path const& path, GLVertex* gl_vertex);

 private:
  void HandleMoveTo(Point const& pt);
  void HandleLineTo(Point const& from, Point const& to);
  void HandleQuadTo(Point const& start, Point const& control, Point const& end);
  void HandleConicTo(Point const& start, Point const& control, Point const& end,
                     float weight);
  void HandleCubicTo(Point const& start, Point const& control1,
                     Point const& control2, Point const& end);
  void HandleClose();

  void HandleCapIfNeed();

  void HandleCap(Point const& point, Vector const& outer_dir);

  void HandleBevelJoin(Point const& from, Point const& to,
                       int32_t prev_pt1_index, int32_t prev_pt2_index);

  void HandleMiterJoin(Point const& from, Point const& to,
                       Vector const& vertical_line);

  bool HandleRoundJoin(Point const& from, Point const& to,
                       Vector const& vertical_line, Point const& from_pt1,
                       Point const& from_pt2);

  void AppendQuadOrSplitRecursively(std::array<Point, 3> const& outer,
                                    std::array<Point, 3> const& inner);

  void AppendAAQuadRecursively(std::array<Point, 3> const& quad, bool on);

 private:
  float stroke_width_;
  float stroke_radius_;
  float miter_limit_;
  Paint::Cap cap_;
  Paint::Join join_;
  GLVertex* gl_vertex_;
  Point start_pt_ = {};
  Point start_to_ = {};
  Point prev_to_pt_ = {};
  Point prev_fromt_pt_ = {};
  Vector prev_dir_ = {};
  Point prev_pt1_ = {};
  Point prev_pt2_ = {};
  Point first_pt1_;
  Point first_pt2_;
  bool join_last_ = false;
  bool is_anti_alias_ = false;
  float anti_alias_width_ = 0.f;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_STROKE_HPP