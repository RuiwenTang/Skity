#include "src/render/gl/gl_stroke.hpp"

#include <glm/gtx/transform.hpp>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
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
      HandleMiterJoin(from, to, vertical_line);
    } else if (join_ == Paint::kRound_Join) {
      // handle rond join
      Orientation orientation = CalculateOrientation(prev_fromt_pt_, from, to);
      if (orientation != Orientation::kLinear) {
        glm::vec4 prev_vertical_line =
            glm::vec4(prev_dir_.y, -prev_dir_.x, 0, 0);
        Point prev_join1_pt = prev_to_pt_ + prev_vertical_line * stroke_radius_;
        Point prev_join2_pt = prev_to_pt_ - prev_vertical_line * stroke_radius_;

        Point before_join, after_join;
        if (orientation == Orientation::kClockWise) {
          before_join = prev_join2_pt;
          after_join = fromt_pt2;
        } else {
          before_join = prev_join1_pt;
          after_join = fromt_pt1;
        }
        Vector dir = glm::normalize(after_join - before_join);
        Vector outer = Vector(-dir.y, dir.x, 0, 0);
        if (orientation == Orientation::kAntiClockWise) {
          outer = -outer;
        }
        Point outer_p;
        outer_p.x = from.x + outer.x * stroke_radius_ * 2.f;
        outer_p.y = from.y + outer.y * stroke_radius_ * 2.f;
        int32_t p1 = gl_vertex_->AddPoint(before_join.x, before_join.y,
                                          GLVertex::GL_VERTEX_TYPE_RADIUS,
                                          from.x, from.y);
        int32_t p2 = gl_vertex_->AddPoint(after_join.x, after_join.y,
                                          GLVertex::GL_VERTEX_TYPE_RADIUS,
                                          from.x, from.y);
        int32_t p3 = gl_vertex_->AddPoint(outer_p.x, outer_p.y,
                                          GLVertex::GL_VERTEX_TYPE_RADIUS,
                                          from.x, from.y);

        gl_vertex_->AddFront(p1, p2, p3);

        int32_t n_p1 =
            gl_vertex_->AddPoint(before_join.x, before_join.y,
                                 GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
        int32_t n_p2 =
            gl_vertex_->AddPoint(after_join.x, after_join.y,
                                 GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
        int32_t n_c = gl_vertex_->AddPoint(
            from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

        gl_vertex_->AddFront(n_p1, n_p2, n_c);
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

void GLStroke::HandleMiterJoin(Point const& from, Point const& to,
                               Vector const& vertical_line) {
  glm::vec4 prev_vertical_line = glm::vec4(prev_dir_.y, -prev_dir_.x, 0, 0);
  Point outer;
  Matrix matrix_pre;
  Matrix matrix_cur;
  Point p1 = Point(prev_fromt_pt_.x, prev_fromt_pt_.y, 0, 1);
  Point p2 = Point(from.x, from.y, 0, 1);
  Point p3 = Point(to.x, to.y, 0, 1);
  Point p4 = Point(from.x, from.y, 0, 1);
  int32_t ret;

  Orientation orientation = CalculateOrientation(prev_fromt_pt_, from, to);
  if (orientation == Orientation::kLinear) {
    // same direction do nothing
  } else if (orientation == Orientation::kClockWise) {
    matrix_pre =
        glm::translate(glm::vec3{-prev_vertical_line.x * stroke_radius_,
                                 -prev_vertical_line.y * stroke_radius_, 0});
    matrix_cur =
        glm::translate(glm::vec3{-vertical_line.x * stroke_radius_,
                                 -vertical_line.y * stroke_radius_, 0});
  } else {
    matrix_pre =
        glm::translate(glm::vec3{prev_vertical_line.x * stroke_radius_,
                                 prev_vertical_line.y * stroke_radius_, 0});
    matrix_cur = glm::translate(glm::vec3{vertical_line.x * stroke_radius_,
                                          vertical_line.y * stroke_radius_, 0});
  }

  p1 = matrix_pre * p1;
  p2 = matrix_pre * p2;
  p3 = matrix_cur * p3;
  p4 = matrix_cur * p4;
  ret = IntersectLineLine(p1, p2, p3, p4, outer);
  float miter_length =
      glm::length(glm::vec2(from.x - outer.x, from.y - outer.y));
  if (ret == 2 || ret == 0) {
    // parallel do nothing
  } else if (miter_length < 4.5f * stroke_radius_) {
    // TODO calculate miter limit
    int32_t p2_index =
        gl_vertex_->AddPoint(p2.x, p2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    int32_t p4_index =
        gl_vertex_->AddPoint(p4.x, p4.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    int32_t outer_index = gl_vertex_->AddPoint(
        outer.x, outer.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    gl_vertex_->AddFront(p2_index, outer_index, p4_index);

    int32_t center_index = gl_vertex_->AddPoint(
        from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    gl_vertex_->AddFront(p2_index, center_index, p4_index);
  } else {
    // fallback bevel_join
    int32_t p2_index =
        gl_vertex_->AddPoint(p2.x, p2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    int32_t p4_index =
        gl_vertex_->AddPoint(p4.x, p4.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    int32_t center_index = gl_vertex_->AddPoint(
        from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    gl_vertex_->AddFront(p2_index, center_index, p4_index);
  }
}

}  // namespace skity