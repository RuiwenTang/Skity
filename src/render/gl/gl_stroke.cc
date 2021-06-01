#include "src/render/gl/gl_stroke.hpp"

#include "src/render/gl/gl_path_visitor.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLStroke::GLStroke(Paint const& paint)
    : stroke_width_(paint.getStrokeWidth()),
      stroke_radius_(stroke_width_ / 2.f),
      miter_limit_(paint.getStrokeMiter()),
      cap_(paint.getStrokeCap()),
      join_(paint.getStrokeJoin()),
      gl_vertex_(nullptr) {}

void GLStroke::strokePath(Path const& path, GLVertex* gl_vertex) {
  Path::Iter iter{path, false};
  gl_vertex_ = gl_vertex;
  std::array<Point, 4> pts;
  for (;;) {
    Path::Verb verb = iter.next(pts.data());
    switch (verb) {
      case Path::Verb::kMove:
        HandleMoveTo(pts[0]);
        break;
      case Path::Verb::kLine:
        HandleLineTo(pts[0], pts[1]);
        break;
      case Path::Verb::kQuad:
        HandleQuadTo(pts[0], pts[1], pts[2]);
        break;
      case Path::Verb::kConic:
        HandleConicTo(pts[0], pts[1], pts[2], iter.conicWeight());
        break;
      case Path::Verb::kCubic:
        HandleCubicTo(pts[0], pts[1], pts[2], pts[3]);
        break;
      case Path::Verb::kClose:
        HandleClose();
        break;
      case Path::Verb::kDone:
        goto DONE;
        break;
    }
  }
DONE:
  return;
}

void GLStroke::HandleMoveTo(Point const& pt) { prev_pt_ = pt; }

void GLStroke::HandleLineTo(Point const& from, Point const& to) {
  glm::vec4 curr_dir = glm::normalize(to - from);
  glm::vec4 vertical_line = glm::vec4(curr_dir.y, -curr_dir.x, 0, 0);
  if (prev_pt_ == from) {
    // first line_to
    first_pt1_ = prev_pt1_ = from + vertical_line * (stroke_radius_);
    first_pt2_ = prev_pt2_ = from - vertical_line * (stroke_radius_);
    prev_pt1_index_ = gl_vertex_->AddPoint(
        first_pt1_.x, first_pt1_.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    prev_pt2_index_ = gl_vertex_->AddPoint(
        first_pt2_.x, first_pt2_.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  } else {
    // handle line join
  }

  Point to_pt1 = to + vertical_line * stroke_radius_;
  Point to_pt2 = to - vertical_line * stroke_radius_;

  int32_t cur_pt1_index = gl_vertex_->AddPoint(
      to_pt1.x, to_pt1.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  int32_t cur_pt2_index = gl_vertex_->AddPoint(
      to_pt2.x, to_pt2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);

  gl_vertex_->AddFront(prev_pt1_index_, cur_pt1_index, cur_pt2_index);
  gl_vertex_->AddFront(prev_pt1_index_, cur_pt2_index, prev_pt2_index_);
}

void GLStroke::HandleQuadTo(Point const& start, Point const& control,
                            Point const& end) {}

void GLStroke::HandleConicTo(Point const& start, Point const& control,
                             Point const& end, float weight) {}

void GLStroke::HandleCubicTo(Point const& start, Point const& control1,
                             Point const& control2, Point const& end) {}

void GLStroke::HandleClose() {}

}  // namespace skity