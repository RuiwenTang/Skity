
#include "src/render/gl/gl_stroke2.hpp"

#include <tuple>

#include "src/geometry/geometry.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLStroke2::GLStroke2(const Paint& paint, GLVertex2* gl_vertex)
    : GLPathVisitor(paint, gl_vertex),
      stroke_radius_(paint.getStrokeWidth() * 0.5f) {}

void GLStroke2::HandleMoveTo(const Point& pt) { first_pt_.Set(Vec2{pt}); }

void GLStroke2::HandleLineTo(const Point& from, const Point& to) {
  auto const& first_pt = *first_pt_;
  Vec2 current_dir = to - from;
  current_dir = glm::normalize(current_dir);
  if (!PointEqualPoint(first_pt, from)) {
    // Handle line_join
    HandleLineJoin(Vec2{from}, Vec2{to});
  } else {
    // save first dir
    first_dir_.Set(current_dir);
  }

  Vec2 normal = Vec2{-current_dir.y, current_dir.x};

  Vec2 from_outer = Vec2{from} + normal * stroke_radius_;
  Vec2 from_inner = Vec2{from} - normal * stroke_radius_;
  Vec2 to_outer = Vec2{to} + normal * stroke_radius_;
  Vec2 to_inner = Vec2{to} - normal * stroke_radius_;

  auto a = GetGLVertex()->AddPoint(
      from_outer.x, from_outer.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 1.f, 0.f);
  auto b = GetGLVertex()->AddPoint(
      from_inner.x, from_inner.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, -1.f, 0.f);

  auto c = GetGLVertex()->AddPoint(
      to_outer.x, to_outer.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 1.f, 0.f);

  auto d = GetGLVertex()->AddPoint(
      to_inner.x, to_inner.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, -1.f, 0.f);

  GetGLVertex()->AddFront(a, b, c);
  GetGLVertex()->AddFront(b, d, c);

  // end
  prev_dir_.Set(current_dir);
  prev_pt_.Set(Vec2{from});
  cur_pt_.Set(Vec2{to});
  cur_dir_.Set(current_dir);
}

void GLStroke2::HandleQuadTo(const Point& from, const Point& control,
                             const Point& end) {
  Vec2 from_vec2 = Vec2{from};
  Vec2 control_vec2 = Vec2{control};
  Vec2 end_vec2 = Vec2{end};

  Orientation orientation =
      CalculateOrientation(from_vec2, control_vec2, end_vec2);

  if (orientation == Orientation::kLinear) {
    // fallback lineTo
    HandleLineTo(from, end);
    return;
  }

  Vec2 first_pt = *first_pt_;
  Vec2 current_dir = glm::normalize(control_vec2 - from_vec2);
  Vec2 end_dir = glm::normalize(end_vec2 - control_vec2);

  if (!PointEqualPoint(first_pt, from_vec2)) {
    // Handle line_join
    HandleLineJoin(from_vec2, control_vec2);
  } else {
    first_dir_.Set(current_dir);
  }

  Vec2 p1, p2, p3, p4, c;

  Vec2 cur_nor = Vec2{-current_dir.y, current_dir.x};
  Vec2 end_nor = Vec2{-end_dir.y, end_dir.x};

  p1 = from_vec2 - cur_nor * stroke_radius_;
  p2 = from_vec2 + cur_nor * stroke_radius_;
  p3 = end_vec2 - end_nor * stroke_radius_;
  p4 = end_vec2 + end_nor * stroke_radius_;

  if (orientation == Orientation::kClockWise) {
    std::swap(p1, p2);
    std::swap(p3, p4);
  }

  IntersectLineLine(p1, p1 + current_dir * stroke_radius_, p3,
                    p3 - end_dir * stroke_radius_, c);

  float type = IsAntiAlias() ? GLVertex2::STROKE_QUAD : GLVertex2::NONE;

  Vec2 C = from_vec2;
  Vec2 P1 = control_vec2;
  Vec2 P2 = end_vec2;
  Vec2 B = Times2(P1 - C);
  Vec2 A = P2 - Times2(P1) + C;

  Vec2 p1_pq, p2_pq, p3_pq, p4_pq, c_pq;

  float dot_AA = -2.f * glm::dot(A, A);
  float dot_AB = -3.f * glm::dot(A, B);
  float offset = -dot_AB / (3.f * dot_AA);

  CalculateQuadPQ(A, B, C, p1, p1_pq);
  CalculateQuadPQ(A, B, C, p2, p2_pq);
  CalculateQuadPQ(A, B, C, p3, p3_pq);
  CalculateQuadPQ(A, B, C, p4, p4_pq);
  CalculateQuadPQ(A, B, C, c, c_pq);

  auto p1_index = GetGLVertex()->AddPoint(p1.x, p1.y, type, p1_pq.x, p1_pq.y);
  auto p2_index = GetGLVertex()->AddPoint(p2.x, p2.y, type, p2_pq.x, p2_pq.y);
  auto p3_index = GetGLVertex()->AddPoint(p3.x, p3.y, type, p3_pq.x, p3_pq.y);
  auto p4_index = GetGLVertex()->AddPoint(p4.x, p4.y, type, p4_pq.x, p4_pq.y);
  auto c_index = GetGLVertex()->AddPoint(c.x, c.y, type, c_pq.x, c_pq.y);

  auto quad_start = GetGLVertex()->QuadCount();

  GetGLVertex()->AddQuad(p1_index, c_index, p3_index);
  GetGLVertex()->AddQuad(p1_index, p2_index, p4_index);
  GetGLVertex()->AddQuad(p1_index, p4_index, p3_index);

  auto quad_count = GetGLVertex()->QuadCount() - quad_start;

  quad_range_.emplace_back(quad_start, quad_count, A, B, C, offset);

  // save
  prev_dir_.Set(end_dir);
  prev_pt_.Set(control_vec2);
  cur_pt_.Set(end_vec2);
  cur_dir_.Set(end_dir);
}

void GLStroke2::HandleClose() { HandleFirstAndEndCap(); }

void GLStroke2::HandleFinish(GLMeshRange* range) {


  if (quad_range_.empty()) {
    return;
  }

  range->quad_front_range.insert(range->quad_front_range.end(),
                                 quad_range_.begin(), quad_range_.end());

  quad_range_.clear();

  if (*cur_pt_ == *first_pt_) {
    return;
  }

  Vec2 curr_dir = glm::normalize(*cur_pt_ - *prev_pt_);

  switch (GetCap()) {
    case Paint::kSquare_Cap:
      HandleSquareCapInternal(*first_pt_, *first_dir_);
      HandleSquareCapInternal(*cur_pt_, -curr_dir);
      break;
    case Paint::kButt_Cap:
      HandleButtCapInternal(*first_pt_, *first_dir_);
      HandleButtCapInternal(*cur_pt_, -curr_dir);
      break;
    case Paint::kRound_Cap:
      HandleRoundCapInternal(*first_pt_, *first_dir_);
      HandleRoundCapInternal(*cur_pt_, -curr_dir);
      break;
    default:
      break;
  }
}

void GLStroke2::HandleFirstAndEndCap() {
  if (!cur_pt_.IsValid() || !first_pt_.IsValid()) {
    return;
  }

  if (*cur_pt_ == *first_pt_) {
    // TODO handle first and last join
    Vec2 f_pt = *first_pt_;
    Vec2 f_dir = *first_dir_;
    Vec2 f_nor = Vec2{-f_dir.y, f_dir.x};
    Vec2 cur_pt = *cur_pt_;
    Vec2 cur_dir = *cur_dir_;
    Vec2 cur_nor = Vec2{-cur_dir.y, cur_dir.x};

    Orientation orientation =
        CalculateOrientation(*prev_pt_, cur_pt, cur_pt + f_dir);

    if (orientation == Orientation::kLinear) {
      // no need handle join
      return;
    }

    Vec2 prev_join;
    Vec2 curr_join;

    if (orientation == Orientation::kAntiClockWise) {
      prev_join = cur_pt - cur_nor * stroke_radius_;
      curr_join = cur_pt - f_nor * stroke_radius_;
    } else {
      prev_join = cur_pt + cur_nor * stroke_radius_;
      curr_join = cur_pt + f_nor * stroke_radius_;
    }

    switch (GetJoin()) {
      case Paint::kMiter_Join:
        HandleMiterJoinInternal(f_pt, prev_join, cur_dir, curr_join, f_dir);
        break;
      case Paint::kBevel_Join:
        HandleBevelJoinInternal(f_pt, prev_join, curr_join, Vec2{0, 0});
        break;
      case Paint::kRound_Join:
        HandleRoundJoinInternal(f_pt, prev_join, cur_dir, curr_join, f_dir);
        break;
      default:
        break;
    }

    return;
  }

  Vec2 curr_dir = glm::normalize(*cur_pt_ - *prev_pt_);

  switch (GetCap()) {
    case Paint::kSquare_Cap:
      HandleSquareCapInternal(*first_pt_, *first_dir_);
      HandleSquareCapInternal(*cur_pt_, -curr_dir);
      break;
    case Paint::kButt_Cap:
      HandleButtCapInternal(*first_pt_, *first_dir_);
      HandleButtCapInternal(*cur_pt_, -curr_dir);
      break;
    case Paint::kRound_Cap:
      HandleRoundCapInternal(*first_pt_, *first_dir_);
      HandleRoundCapInternal(*cur_pt_, -curr_dir);
      break;
    default:
      break;
  }
}

void GLStroke2::HandleLineJoin(const Vec2& from, const Vec2& to) {
  auto orientation = CalculateOrientation(*prev_pt_, from, to);
  if (orientation == Orientation::kLinear) {
    return;
  }

  Vec2 prev_normal = Vec2{-(*prev_dir_).y, (*prev_dir_).x};
  Vec2 current_dir = glm::normalize(to - from);
  Vec2 current_normal = Vec2{-current_dir.y, current_dir.x};

  Vec2 prev_join;
  Vec2 curr_join;

  if (orientation == Orientation::kAntiClockWise) {
    prev_join = from - prev_normal * stroke_radius_;
    curr_join = from - current_normal * stroke_radius_;
  } else {
    prev_join = from + prev_normal * stroke_radius_;
    curr_join = from + current_normal * stroke_radius_;
  }

  switch (GetJoin()) {
    case Paint::kMiter_Join:
      HandleMiterJoinInternal(from, prev_join, *prev_dir_, curr_join,
                              -current_dir);
      break;
    case Paint::kBevel_Join:
      HandleBevelJoinInternal(from, prev_join, curr_join,
                              glm::normalize(*prev_dir_ - current_dir));
      break;
    case Paint::kRound_Join:
      HandleRoundJoinInternal(from, prev_join, *prev_dir_, curr_join,
                              current_dir);
      break;
    default:
      break;
  }
}

void GLStroke2::HandleMiterJoinInternal(const Vec2& center, const Vec2& p1,
                                        const Vec2& d1, const Vec2& p2,
                                        const Vec2& d2) {
  Vec2 pp1 = p1 - center;
  Vec2 pp2 = p2 - center;

  Vec2 out_dir = pp1 + pp2;

  float k = 2.f * stroke_radius_ * stroke_radius_ /
            (out_dir.x * out_dir.x + out_dir.y * out_dir.y);

  Vec2 pe = k * out_dir;

  if (glm::length(pe) >= GetMiterLimit()) {
    // fallback to bevel_join
    HandleBevelJoinInternal(center, p1, p2, glm::normalize(out_dir));
    return;
  }

  Vec2 join = center + pe;

  auto c = GetGLVertex()->AddPoint(
      center.x, center.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, -1.f, 0.f);
  auto cp1 = GetGLVertex()->AddPoint(
      p1.x, p1.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 1.f,
      0.f);
  auto cp2 = GetGLVertex()->AddPoint(
      p2.x, p2.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 1.f,
      0.f);

  auto e = GetGLVertex()->AddPoint(
      join.x, join.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE,
      1.f, 0.f);

  GetGLVertex()->AddFront(c, cp1, e);
  GetGLVertex()->AddFront(c, cp2, e);
}

void GLStroke2::HandleBevelJoinInternal(const Vec2& center, const Vec2& p1,
                                        const Vec2& p2, const Vec2& curr_dir) {
  auto type = IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE;

  auto a = GetGLVertex()->AddPoint(center.x, center.y, type, -1.f, 0.f);
  auto b = GetGLVertex()->AddPoint(p1.x, p1.y, type, 1.f, 0.f);
  auto c = GetGLVertex()->AddPoint(p2.x, p2.y, type, 1.f, 0.f);

  GetGLVertex()->AddFront(a, b, c);
}

void GLStroke2::HandleRoundJoinInternal(Vec2 const& center, Vec2 const& p1,
                                        Vec2 const& d1, Vec2 const& p2,
                                        Vec2 const& d2) {
  int type = GLVertex2::LINE_ROUND;
  if (IsAntiAlias()) {
    type += GLVertex2::STROKE_AA;
  }

  Vec2 out_point = center + (d1 - d2) * stroke_radius_;

  auto a =
      GetGLVertex()->AddPoint(center.x, center.y, type, center.x, center.y);
  auto b = GetGLVertex()->AddPoint(p1.x, p1.y, type, center.x, center.y);
  auto c = GetGLVertex()->AddPoint(p2.x, p2.y, type, center.x, center.y);
  auto e = GetGLVertex()->AddPoint(out_point.x, out_point.y, type, center.x,
                                   center.y);

  GetGLVertex()->AddFront(a, b, e);
  GetGLVertex()->AddFront(a, e, c);
}

void GLStroke2::HandleSquareCapInternal(const Vec2& pt, const Vec2& dir) {
  float step = stroke_radius_ * 0.1f;
  if (FloatNearlyZero(step)) {
    return;
  }

  if (!IsAntiAlias()) {
    return;
  }

  Vec2 normal = Vec2{-dir.y, dir.x};

  Vec2 out_p = pt + normal * stroke_radius_;
  Vec2 in_p = pt - normal * stroke_radius_;

  Vec2 out_p_cap = out_p - dir * step;
  Vec2 in_p_cap = in_p - dir * step;

  auto c =
      GetGLVertex()->AddPoint(out_p.x, out_p.y, GLVertex2::LINE_CAP, 1.f, 1.f);
  auto d =
      GetGLVertex()->AddPoint(in_p.x, in_p.y, GLVertex2::LINE_CAP, 1.f, -1.f);

  auto a = GetGLVertex()->AddPoint(out_p_cap.x, out_p_cap.y,
                                   GLVertex2::LINE_CAP, 0.f, 1.f);
  auto b = GetGLVertex()->AddPoint(in_p_cap.x, in_p_cap.y, GLVertex2::LINE_CAP,
                                   0.f, -1.f);

  GetGLVertex()->AddFront(a, b, d);
  GetGLVertex()->AddFront(a, d, c);
}

void GLStroke2::HandleButtCapInternal(const Vec2& pt, const Vec2& dir) {
  // Step 1 build Butt rect point
  Vec2 normal = Vec2{-dir.y, dir.x};

  Vec2 out_p = pt + normal * stroke_radius_;
  Vec2 in_p = pt - normal * stroke_radius_;

  Vec2 out_p_butt = out_p - dir * stroke_radius_;
  Vec2 in_p_butt = in_p - dir * stroke_radius_;

  auto a = GetGLVertex()->AddPoint(
      out_p_butt.x, out_p_butt.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 1.f, 0.f);

  auto b = GetGLVertex()->AddPoint(
      in_p_butt.x, in_p_butt.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, -1.f, 0.f);

  auto c = GetGLVertex()->AddPoint(
      out_p.x, out_p.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE,
      1.f, 0.f);

  auto d = GetGLVertex()->AddPoint(
      in_p.x, in_p.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE,
      -1.f, 0.f);

  GetGLVertex()->AddFront(a, b, d);
  GetGLVertex()->AddFront(a, d, c);

  // Step 2 build aa square if needed
  if (IsAntiAlias()) {
    Vec2 fake_pt = pt - dir * stroke_radius_;

    HandleSquareCapInternal(fake_pt, dir);
  }
}

void GLStroke2::HandleRoundCapInternal(Vec2 const& pt, Vec2 const& dir) {
  int32_t circle_type = GLVertex2::LINE_ROUND;
  if (IsAntiAlias()) {
    circle_type += GLVertex2::STROKE_AA;
  }

  Vec2 normal = {-dir.y, dir.x};

  Vec2 p1 = pt + normal * stroke_radius_;
  Vec2 p2 = pt - normal * stroke_radius_;
  Vec2 p3 = p1 - dir * stroke_radius_;
  Vec2 p4 = p2 - dir * stroke_radius_;

  auto a = GetGLVertex()->AddPoint(p1.x, p1.y, circle_type, pt.x, pt.y);
  auto b = GetGLVertex()->AddPoint(p2.x, p2.y, circle_type, pt.x, pt.y);
  auto c = GetGLVertex()->AddPoint(p3.x, p3.y, circle_type, pt.x, pt.y);
  auto d = GetGLVertex()->AddPoint(p4.x, p4.y, circle_type, pt.x, pt.y);

  GetGLVertex()->AddFront(a, b, c);
  GetGLVertex()->AddFront(b, d, c);
}

}  // namespace skity
