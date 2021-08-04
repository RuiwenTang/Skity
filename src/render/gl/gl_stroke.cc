#include "src/render/gl/gl_stroke.hpp"

#include <glm/gtx/transform.hpp>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLStroke::GLStroke(Paint const& paint)
    : stroke_width_(paint.getStrokeWidth()),
      stroke_radius_(stroke_width_ / 2.f),
      miter_limit_(paint.getStrokeMiter()),
      cap_(paint.getStrokeCap()),
      join_(paint.getStrokeJoin()),
      gl_vertex_(nullptr),
      is_anti_alias_(paint.isAntiAlias()),
      path_effect_(paint.getPathEffect()) {}

GLMeshRange GLStroke::strokePath(Path const& path, GLVertex* gl_vertex) {
  GLMeshRange range{};
  range.front_start = gl_vertex->FrontCount();
  range.front_count = 0;
  range.back_start = gl_vertex->BackCount();
  range.back_count = 0;
  range.aa_outline_start = gl_vertex->AAOutlineCount();
  range.aa_outline_count = 0;

  const Path* dst;
  Path tmp;
  if (path_effect_ && path_effect_->filterPath(&tmp, path, false)) {
    dst = &tmp;
  } else {
    dst = &path;
  }

  Path::Iter iter(*dst, true);

  gl_vertex_ = gl_vertex;
  std::array<Point, 4> pts;
  bool has_close = false;

  if (is_anti_alias_) {
    anti_alias_width_ = std::min(1.f, stroke_width_ / 2.f);
  }

  for (;;) {
    Path::Verb verb = iter.next(pts.data());
    switch (verb) {
      case Path::Verb::kMove:
        has_close = false;
        HandleCapIfNeed();
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
        has_close = true;
        break;
      case Path::Verb::kDone:
        goto DONE;
        break;
    }
  }
DONE:
  if (!has_close) {
    HandleCapIfNeed();
  }
  range.front_count = gl_vertex->FrontCount() - range.front_start;
  range.back_count = gl_vertex->BackCount() - range.back_start;
  range.aa_outline_count = gl_vertex->AAOutlineCount() - range.aa_outline_start;
  return range;
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
    start_to_ = to;
    join_last_ = true;
  } else {
    // handle line join
    if (join_ == Paint::kBevel_Join) {
      HandleBevelJoin(from, to, prev_pt1_index, prev_pt2_index);
    } else if (join_ == Paint::kMiter_Join) {
      // miter join
      HandleMiterJoin(from, to, vertical_line);
    } else if (join_ == Paint::kRound_Join) {
      // handle rond join
      bool degent =
          HandleRoundJoin(from, to, vertical_line, fromt_pt1, fromt_pt2);

      if (degent) {
        // fallback bevel join
        HandleBevelJoin(from, to, prev_pt1_index, prev_pt2_index);
      }
    }
  }

  gl_vertex_->AddFront(prev_pt1_index, cur_pt1_index, cur_pt2_index);
  gl_vertex_->AddFront(prev_pt1_index, prev_pt2_index, cur_pt2_index);
  prev_to_pt_ = to;
  prev_fromt_pt_ = from;
  prev_dir_ = curr_dir;

  if (is_anti_alias_) {
    Point from_pt1_aa = fromt_pt1 + vertical_line * anti_alias_width_;
    Point from_pt2_aa = fromt_pt2 - vertical_line * anti_alias_width_;
    Point to_pt1_aa = to_pt1 + vertical_line * anti_alias_width_;
    Point to_pt2_aa = to_pt2 - vertical_line * anti_alias_width_;

    uint32_t from_pt1_aa_index =
        gl_vertex_->AddPoint(from_pt1_aa.x, from_pt1_aa.y, 0.f,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    uint32_t from_pt2_aa_index =
        gl_vertex_->AddPoint(from_pt2_aa.x, from_pt2_aa.y, 0.f,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    uint32_t from_pt1_index =
        gl_vertex_->AddPoint(fromt_pt1.x, fromt_pt1.y, 1.f,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    uint32_t from_pt2_index =
        gl_vertex_->AddPoint(fromt_pt2.x, fromt_pt2.y, 1.f,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    uint32_t to_pt1_aa_index =
        gl_vertex_->AddPoint(to_pt1_aa.x, to_pt1_aa.y, 0.f,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    uint32_t to_pt2_aa_index =
        gl_vertex_->AddPoint(to_pt2_aa.x, to_pt2_aa.y, 0.f,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    uint32_t to_pt1_index = gl_vertex_->AddPoint(
        to_pt1.x, to_pt1.y, 1.f, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    uint32_t to_pt2_index = gl_vertex_->AddPoint(
        to_pt2.x, to_pt2.y, 1.f, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    gl_vertex_->AddFront(from_pt1_aa_index, from_pt1_index, to_pt1_index);
    gl_vertex_->AddFront(from_pt1_aa_index, to_pt1_aa_index, to_pt1_index);

    gl_vertex_->AddFront(from_pt2_aa_index, from_pt2_index, to_pt2_index);
    gl_vertex_->AddFront(from_pt2_aa_index, to_pt2_aa_index, to_pt2_index);
  }
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

  Vector outerStart = start_pt1;
  Vector outerStartControl = start_pt1 + start_dir;
  Point outerControl;
  IntersectLineLine(outerStart, outerStartControl, control,
                    control + control_vertical_line, outerControl);

  Vector innerStart = start_pt2;
  Vector innerStartControl = start_pt2 + start_dir;
  Point innerControl;
  IntersectLineLine(innerStart, innerStartControl, control,
                    control - control_vertical_line, innerControl);

  Point control_pt1 = outerControl;
  Point control_pt2 = innerControl;

  int32_t prev_pt1_index = gl_vertex_->AddPoint(
      start_pt1.x, start_pt1.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  int32_t prev_pt2_index = gl_vertex_->AddPoint(
      start_pt2.x, start_pt2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
  if (start_pt_ == start) {
    // first quad to
    first_pt1_ = prev_pt1_ = start_pt1;
    first_pt2_ = prev_pt2_ = start_pt2;
    start_to_ = control;
    join_last_ = true;
  } else {
    // handle line join
    if (join_ == Paint::kBevel_Join) {
      HandleBevelJoin(start, control, prev_pt1_index, prev_pt2_index);
    } else if (join_ == Paint::kMiter_Join) {
      HandleMiterJoin(start, control, start_vertical_line);
    } else if (join_ == Paint::kRound_Join) {
      bool degent = HandleRoundJoin(start, control, start_vertical_line,
                                    start_pt1, start_pt2);
      if (degent) {
        HandleBevelJoin(start, control, prev_pt1_index, prev_pt2_index);
      }
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
                             Point const& end, float weight) {
  std::array<Point, 5> quads;
  Conic conic{start, control, end, weight};
  conic.chopIntoQuadsPOW2(quads.data(), 1);

  Paint::Join save_join = join_;
  join_ = Paint::kRound_Join;
  quads[0] = start;
  HandleQuadTo(quads[0], quads[1], quads[2]);
  HandleQuadTo(quads[2], quads[3], quads[4]);
  join_ = save_join;
}

void GLStroke::HandleCubicTo(Point const& start, Point const& control1,
                             Point const& control2, Point const& end) {
  std::array<Point, 4> cubic{start, control1, control2, end};

  std::array<skity::Point, 32> sub_cubics;
  SubDividedCubic8(cubic.data(), sub_cubics.data());
  Paint::Join save_join = join_;
  join_ = Paint::kRound_Join;
  for (int i = 0; i < 8; i++) {
    std::array<skity::Point, 3> quad;
    skity::CubicToQuadratic(sub_cubics.data() + i * 4, quad.data());
    HandleQuadTo(quad[0], quad[1], quad[2]);
  }
  join_ = save_join;
}

void GLStroke::HandleClose() {
  if (join_last_) {
    glm::vec4 curr_dir = glm::normalize(start_to_ - start_pt_);
    glm::vec4 vertical_line = glm::vec4(curr_dir.y, -curr_dir.x, 0, 0);
    if (join_ == Paint::kBevel_Join) {
      // HandleBevelJoin()
    } else if (join_ == Paint::kMiter_Join) {
      HandleMiterJoin(start_pt_, start_to_, vertical_line);
    } else if (join_ == Paint::kRound_Join) {
      HandleRoundJoin(start_pt_, start_to_, vertical_line, first_pt1_,
                      first_pt2_);
    }
    join_last_ = false;
  }
}

void GLStroke::HandleCapIfNeed() {
  if (start_pt_ == prev_to_pt_) {
    // path is close
    return;
  }

  if (cap_ == Paint::kButt_Cap) {
    // no nothing
    return;
  }

  // cap start
  HandleCap(start_pt_, glm::normalize(start_pt_ - start_to_));
  // cap end
  HandleCap(prev_to_pt_, glm::normalize(prev_to_pt_ - prev_fromt_pt_));
}

void GLStroke::HandleCap(Point const& point, Vector const& outer_dir) {
  Vector vertical_dir = Vector(outer_dir.y, -outer_dir.x, 0, 0);
  Point p_1 = point + vertical_dir * stroke_radius_;
  Point p_2 = point - vertical_dir * stroke_radius_;
  Point p_3 = point + outer_dir * stroke_radius_;

  Point p_1_3 = p_1 + outer_dir * stroke_radius_;
  Point p_2_3 = p_2 + outer_dir * stroke_radius_;

  if (cap_ == Paint::kRound_Cap) {
    // round cap
    int32_t p_index = gl_vertex_->AddPoint(
        point.x, point.y, GLVertex::GL_VERTEX_TYPE_RADIUS, point.x, point.y);
    int32_t p_1_index = gl_vertex_->AddPoint(
        p_1.x, p_1.y, GLVertex::GL_VERTEX_TYPE_RADIUS, point.x, point.y);
    int32_t p_2_index = gl_vertex_->AddPoint(
        p_2.x, p_2.y, GLVertex::GL_VERTEX_TYPE_RADIUS, point.x, point.y);
    int32_t p_3_index = gl_vertex_->AddPoint(
        p_3.x, p_3.y, GLVertex::GL_VERTEX_TYPE_RADIUS, point.x, point.y);
    int32_t p_1_3_index = gl_vertex_->AddPoint(
        p_1_3.x, p_1_3.y, GLVertex::GL_VERTEX_TYPE_RADIUS, point.x, point.y);
    int32_t p_2_3_index = gl_vertex_->AddPoint(
        p_2_3.x, p_2_3.y, GLVertex::GL_VERTEX_TYPE_RADIUS, point.x, point.y);

    gl_vertex_->AddFront(p_index, p_1_index, p_1_3_index);
    gl_vertex_->AddFront(p_index, p_1_3_index, p_3_index);
    gl_vertex_->AddFront(p_index, p_3_index, p_2_3_index);
    gl_vertex_->AddFront(p_index, p_2_3_index, p_2_index);
  } else {
    // square cap
    int32_t p_1_index = gl_vertex_->AddPoint(
        p_1.x, p_1.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t p_1_3_index = gl_vertex_->AddPoint(
        p_1_3.x, p_1_3.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t p_2_index = gl_vertex_->AddPoint(
        p_2.x, p_2.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t p_2_3_index = gl_vertex_->AddPoint(
        p_2_3.x, p_2_3.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    gl_vertex_->AddFront(p_1_index, p_1_3_index, p_2_3_index);
    gl_vertex_->AddFront(p_1_index, p_2_3_index, p_2_index);
  }
}

void GLStroke::HandleBevelJoin(Point const& from, Point const& to,
                               int32_t prev_pt1_index, int32_t prev_pt2_index) {
  glm::vec4 prev_vertical_line = glm::vec4(prev_dir_.y, -prev_dir_.x, 0, 0);
  Point prev_join1_pt = prev_to_pt_ + prev_vertical_line * stroke_radius_;
  Point prev_join2_pt = prev_to_pt_ - prev_vertical_line * stroke_radius_;
  Orientation orientation = CalculateOrientation(prev_fromt_pt_, from, to);

  if (orientation != Orientation::kAntiClockWise) {
    int32_t prev_join_index =
        gl_vertex_->AddPoint(prev_join2_pt.x, prev_join2_pt.y,
                             GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    // bevel join
    int32_t center_point = gl_vertex_->AddPoint(
        from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0, 0);
    gl_vertex_->AddFront(prev_join_index, prev_pt2_index, center_point);
  } else {
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
  } else if (miter_length < miter_limit_) {
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

bool GLStroke::HandleRoundJoin(Point const& from, Point const& to,
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
  } else {
    return true;
  }

  return false;
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

    if (is_anti_alias_) {
      AppendAAQuadRecursively(outer, true);
      AppendAAQuadRecursively(inner, false);
    }
  }
}

void GLStroke::AppendAAQuadRecursively(std::array<Point, 3> const& quad,
                                       bool on) {
  if (CalculateOrientation(quad[0], quad[1], quad[2]) != Orientation::kLinear) {
    std::array<Point, 3> quad_1{};
    std::array<Point, 3> quad_2{};
    SubDividedQuad(quad.data(), quad_1.data(), quad_2.data());

    AppendAAQuadRecursively(quad_1, on);
    AppendAAQuadRecursively(quad_2, on);
  } else {
    // TODO handle on off direction
    Point from = quad[0];
    Point to = quad[2];
    glm::vec2 curr_dir = glm::normalize(glm::vec2(to - from));
    glm::vec4 vertical_line = glm::vec4(curr_dir.y, -curr_dir.x, 0, 0);

    Point from_1 = from + vertical_line * anti_alias_width_;
    Point from_2 = from - vertical_line * anti_alias_width_;
    Point to_1 = to + vertical_line * anti_alias_width_;
    Point to_2 = to - vertical_line * anti_alias_width_;

    int32_t from_index = gl_vertex_->AddPoint(
        from.x, from.y, 1.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);
    int32_t to_index = gl_vertex_->AddPoint(
        to.x, to.y, 1.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);

    int32_t from_1_index = gl_vertex_->AddPoint(
        from_1.x, from_1.y, 0.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);
    int32_t from_2_index = gl_vertex_->AddPoint(
        from_2.x, from_2.y, 0.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);

    int32_t to_1_index = gl_vertex_->AddPoint(
        to_1.x, to_1.y, 0.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);
    int32_t to_2_index = gl_vertex_->AddPoint(
        to_2.x, to_2.y, 0.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);

    gl_vertex_->AddAAOutline(from_1_index, from_index, to_index);
    gl_vertex_->AddAAOutline(from_1_index, to_index, to_1_index);

    gl_vertex_->AddAAOutline(from_2_index, from_index, to_index);
    gl_vertex_->AddAAOutline(from_2_index, to_index, to_2_index);
  }
}

}  // namespace skity