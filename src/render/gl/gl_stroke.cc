#include "src/render/gl/gl_stroke.hpp"

#include "src/geometry/math.hpp"
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

void GLStroke::HandleMoveTo(Point const& pt) { start_pt_ = pt; }

void GLStroke::HandleLineTo(Point const& from, Point const& to) {
  glm::vec4 curr_dir = glm::normalize(to - from);
  glm::vec4 vertical_line = glm::vec4(curr_dir.y, -curr_dir.x, 0, 0);
  Point fromt_pt1 = from + vertical_line * stroke_radius_;
  Point fromt_pt2 = from - vertical_line * stroke_radius_;

  Point to_pt1 = to + vertical_line * stroke_radius_;
  Point to_pt2 = to - vertical_line * stroke_radius_;

  int32_t prev_pt1_index = gl_vertex_->AddPoint(
      fromt_pt1.x, fromt_pt1.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  int32_t prev_pt2_index = gl_vertex_->AddPoint(
      fromt_pt2.x, fromt_pt2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  int32_t cur_pt1_index = gl_vertex_->AddPoint(
      to_pt1.x, to_pt1.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  int32_t cur_pt2_index = gl_vertex_->AddPoint(
      to_pt2.x, to_pt2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);

  if (start_pt_ == from) {
    // first line_to
    first_pt1_ = prev_pt1_ = from + vertical_line * (stroke_radius_);
    first_pt2_ = prev_pt2_ = from - vertical_line * (stroke_radius_);
  } else {
    // handle line join
    if (join_ == Paint::kBevel_Join) {
      HandleBevelJoin(from, to, prev_pt1_index, prev_pt2_index);
    } else if (join_ == Paint::kMiter_Join) {
      // miter join
      glm::vec4 prev_vertical_line = glm::vec4(prev_dir_.y, -prev_dir_.x, 0, 0);
      float dot_product = glm::dot(vertical_line, prev_vertical_line);
      if (FloatNearlyZero(Float1 - dot_product)) {
        // do nothing
      } else if (FloatNearlyZero(Float1 + dot_product)) {
        // oppsite line
      } else {
      }
    }
  }

  gl_vertex_->AddFront(prev_pt1_index, cur_pt1_index, cur_pt2_index);
  gl_vertex_->AddFront(prev_pt1_index, prev_pt2_index, cur_pt2_index);
  prev_to_pt_ = to;
  prev_fromt_pt_ = from;
  prev_dir_ = curr_dir;
}

void GLStroke::HandleQuadTo(Point const& start, Point const& control,
                            Point const& end) {}

void GLStroke::HandleConicTo(Point const& start, Point const& control,
                             Point const& end, float weight) {}

void GLStroke::HandleCubicTo(Point const& start, Point const& control1,
                             Point const& control2, Point const& end) {}

void GLStroke::HandleClose() {}

void GLStroke::HandleBevelJoin(Point const& from, Point const& to,
                               int32_t prev_pt1_index, int32_t prev_pt2_index) {
  glm::vec4 prev_vertical_line = glm::vec4(prev_dir_.y, -prev_dir_.x, 0, 0);
  Point prev_join1_pt = prev_to_pt_ + prev_vertical_line * stroke_radius_;
  Point prev_join2_pt = prev_to_pt_ - prev_vertical_line * stroke_radius_;
  Orientation orientation = CalculateOrientation(prev_fromt_pt_, from, to);

  if (orientation == Orientation::kLinear) {
    // no need to handle join
  } else if (orientation == Orientation::kClockWise) {
    int32_t prev_join_index =
        gl_vertex_->AddPoint(prev_join2_pt.x, prev_join2_pt.y,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    // bevel join
    int32_t center_point = gl_vertex_->AddPoint(
        from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    gl_vertex_->AddFront(prev_join_index, prev_pt2_index, center_point);
  } else if (orientation == Orientation::kAntiClockWise) {
    int32_t prev_join_index =
        gl_vertex_->AddPoint(prev_join1_pt.x, prev_join1_pt.y,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    int32_t center_point = gl_vertex_->AddPoint(
        from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);

    gl_vertex_->AddFront(prev_join_index, prev_pt1_index, center_point);
  }
}

}  // namespace skity