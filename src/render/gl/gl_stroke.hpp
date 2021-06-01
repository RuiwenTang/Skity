#ifndef SKITY_SRC_RENDER_GL_GL_STROKE_HPP
#define SKITY_SRC_RENDER_GL_GL_STROKE_HPP

#include "src/render/stroke.hpp"

namespace skity {

class GLVertex;

/**
 * @class GLStroke
 *  Used to generate Mesh for path stroke
 *  @todo generate vertex during stroke, not at the end
 */
class GLStroke {
 public:
  explicit GLStroke(Paint const& paint);
  ~GLStroke() = default;

  void strokePath(Path const& path, GLVertex* gl_vertex);

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
  float stroke_width_;
  float stroke_radius_;
  float miter_limit_;
  Paint::Cap cap_;
  Paint::Join join_;
  GLVertex* gl_vertex_;
  Point prev_pt_ = {};
  Point prev_pt1_ = {};
  Point prev_pt2_ = {};
  Point first_pt1_;
  Point first_pt2_;
  int32_t first_pt1_index_ = -1;
  int32_t first_pt2_index_ = -1;
  int32_t prev_pt1_index_ = -1;
  int32_t prev_pt2_index_ = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_STROKE_HPP