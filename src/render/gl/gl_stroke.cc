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
      HandleRoundJoin(from, to, vertical_line, fromt_pt1, fromt_pt2);
    }
  }

  gl_vertex_->AddFront(prev_pt1_index, cur_pt1_index, cur_pt2_index);
  gl_vertex_->AddFront(prev_pt1_index, prev_pt2_index, cur_pt2_index);
  prev_to_pt_ = to;
  prev_fromt_pt_ = from;
  prev_dir_ = curr_dir;
}

void GLStroke::HandleQuadTo(Point const& start, Point const& control,
                            Point const& end) {
  Orientation orientation = CalculateOrientation(start, control, end);

  if (orientation == Orientation::kLinear) {
    // fall back to line to
    HandleLineTo(start, end);
    return;
  }

  Vector start_dir = glm::normalize(control - start);
  Vector start_vertical_line = Vector(start_dir.y, -start_dir.x, 0, 0);
  Point start_pt1 = start + start_vertical_line * stroke_radius_;
  Point start_pt2 = start - start_vertical_line * stroke_radius_;

  Vector end_dir = glm::normalize(end - control);
  Vector end_vertical_line = Vector(end_dir.y, -end_dir.x, 0, 0);
  Point end_pt1 = end + end_vertical_line * stroke_radius_;
  Point end_pt2 = end - end_vertical_line * stroke_radius_;

  Vector control_dir = glm::normalize(start_dir + end_dir);
  Vector control_vertical_line = Vector(control_dir.y, -control_dir.x, 0, 0);
  Point control_pt1 =
      control + control_vertical_line * (stroke_radius_ * 2.25f);
  Point control_pt2 =
      control - control_vertical_line * (stroke_radius_ * 2.25f);

  int32_t prev_pt1_index = gl_vertex_->AddPoint(
      start_pt1.x, start_pt1.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  int32_t prev_pt2_index = gl_vertex_->AddPoint(
      start_pt2.x, start_pt2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  if (start_pt_ == start) {
    // first quad to
    first_pt1_ = prev_pt1_ = start_pt1;
    first_pt2_ = prev_pt2_ = start_pt2;
  } else {
    // handle line join
    if (join_ == Paint::kBevel_Join) {
      HandleBevelJoin(start, control, prev_pt1_index, prev_pt2_index);
    } else if (join_ == Paint::kMiter_Join) {
      HandleMiterJoin(start, control, start_vertical_line);
    } else if (join_ == Paint::kRound_Join) {
      HandleRoundJoin(start, control, start_vertical_line, start_pt1,
                      start_pt2);
    }
  }

  std::array<Point, 3> outer_quad;
  std::array<Point, 3> inner_quad;

  if (orientation == Orientation::kAntiClockWise) {
    outer_quad[0] = start_pt1;
    outer_quad[1] = control_pt1;
    outer_quad[2] = end_pt1;

    inner_quad[0] = start_pt2;
    inner_quad[1] = control_pt2;
    inner_quad[2] = end_pt2;
  } else {
    inner_quad[0] = start_pt1;
    inner_quad[1] = control_pt1;
    inner_quad[2] = end_pt1;

    outer_quad[0] = start_pt2;
    outer_quad[1] = control_pt2;
    outer_quad[2] = end_pt2;
  }

  // need to split quad until no overlap
  AppendQuadOrSplitRecursively(outer_quad, inner_quad);

  prev_to_pt_ = end;
  prev_fromt_pt_ = control;
  prev_dir_ = end_dir;
}

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

void GLStroke::HandleRoundJoin(Point const& from, Point const& to,
                               Vector const& vertical_line,
                               Point const& from_pt1, Point const& from_pt2) {
  Orientation orientation = CalculateOrientation(prev_fromt_pt_, from, to);
  if (orientation != Orientation::kLinear) {
    glm::vec4 prev_vertical_line = glm::vec4(prev_dir_.y, -prev_dir_.x, 0, 0);
    Point prev_join1_pt = prev_to_pt_ + prev_vertical_line * stroke_radius_;
    Point prev_join2_pt = prev_to_pt_ - prev_vertical_line * stroke_radius_;

    Point before_join, after_join;
    if (orientation == Orientation::kClockWise) {
      before_join = prev_join2_pt;
      after_join = from_pt2;
    } else {
      before_join = prev_join1_pt;
      after_join = from_pt1;
    }
    Vector dir = glm::normalize(after_join - before_join);
    Vector before_dir = before_join - from;
    Vector after_dir = after_join - from;
    float cross_prod = CrossProduct(before_dir, after_dir);
    Vector outer = Vector(-dir.y, dir.x, 0, 0);
    if (orientation == Orientation::kAntiClockWise) {
      outer = -outer;
    }
    Point outer_p;
    outer_p.x = from.x + outer.x * stroke_radius_ * 2.f;
    outer_p.y = from.y + outer.y * stroke_radius_ * 2.f;
    if (cross_prod >= 0) {
      int32_t p1 =
          gl_vertex_->AddPoint(before_join.x, before_join.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);
      int32_t p2 =
          gl_vertex_->AddPoint(after_join.x, after_join.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);
      int32_t p3 =
          gl_vertex_->AddPoint(outer_p.x, outer_p.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);

      gl_vertex_->AddFront(p1, p2, p3);
    } else {
      Point i_center;
      IntersectLineLine(before_join, after_join, from, from + outer, i_center);

      int32_t p1 =
          gl_vertex_->AddPoint(before_join.x, before_join.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);
      int32_t p2 =
          gl_vertex_->AddPoint(after_join.x, after_join.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);

      int32_t p3 =
          gl_vertex_->AddPoint(outer_p.x, outer_p.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);

      int32_t center_index =
          gl_vertex_->AddPoint(i_center.x, i_center.y,
                               GLVertex::GL_VERTEX_TYPE_RADIUS, from.x, from.y);

      gl_vertex_->AddFront(p1, p3, center_index);
      gl_vertex_->AddFront(p3, p2, center_index);
    }

    int32_t n_p1 = gl_vertex_->AddPoint(before_join.x, before_join.y,
                                        GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    int32_t n_p2 = gl_vertex_->AddPoint(
        after_join.x, after_join.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t n_c = gl_vertex_->AddPoint(
        from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    gl_vertex_->AddFront(n_p1, n_p2, n_c);
  }
}

void GLStroke::AppendQuadOrSplitRecursively(std::array<Point, 3> const& outer,
                                            std::array<Point, 3> const& inner) {
  if (PointInTriangle(inner[1], outer[0], outer[1], outer[2])) {
    // overlap need split
    std::array<Point, 3> outer_1;
    std::array<Point, 3> outer_2;
    std::array<Point, 3> inner_1;
    std::array<Point, 3> inner_2;
    // TODO maybe inner no need to be subdivided
    SubDividedQuad(outer.data(), outer_1.data(), outer_2.data());
    SubDividedQuad(inner.data(), inner_1.data(), inner_2.data());

    AppendQuadOrSplitRecursively(outer_1, inner_1);
    AppendQuadOrSplitRecursively(outer_2, inner_2);
  } else {
    // add quad triangle
    int32_t o_p1 = gl_vertex_->AddPoint(
        outer[0].x, outer[0].y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.f, 0.f);
    int32_t o_p2 = gl_vertex_->AddPoint(
        outer[1].x, outer[1].y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.5f, 0.f);
    int32_t o_p3 = gl_vertex_->AddPoint(
        outer[2].x, outer[2].y, GLVertex::GL_VERTEX_TYPE_QUAD, 1.f, 1.f);

    int32_t i_p1 = gl_vertex_->AddPoint(
        inner[0].x, inner[0].y, GLVertex::GL_VERTEX_TYPE_QUAD_OFF, 0.f, 0.f);
    int32_t i_p2 = gl_vertex_->AddPoint(
        inner[1].x, inner[1].y, GLVertex::GL_VERTEX_TYPE_QUAD_OFF, 0.5f, 0.f);
    int32_t i_p3 = gl_vertex_->AddPoint(
        inner[2].x, inner[2].y, GLVertex::GL_VERTEX_TYPE_QUAD_OFF, 1.f, 1.f);

    gl_vertex_->AddFront(o_p1, o_p2, o_p3);
    gl_vertex_->AddFront(i_p1, i_p2, i_p3);

    // add normal triangle
    int32_t on_p1 = gl_vertex_->AddPoint(
        outer[0].x, outer[0].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t on_p2 = gl_vertex_->AddPoint(
        outer[2].x, outer[2].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    int32_t in_p1 = gl_vertex_->AddPoint(
        inner[0].x, inner[0].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t in_p2 = gl_vertex_->AddPoint(
        inner[1].x, inner[1].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t in_p3 = gl_vertex_->AddPoint(
        inner[2].x, inner[2].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    gl_vertex_->AddFront(on_p1, in_p1, in_p2);
    gl_vertex_->AddFront(on_p1, in_p2, on_p2);
    gl_vertex_->AddFront(on_p2, in_p2, in_p3);
  }
}

}  // namespace skity