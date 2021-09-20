
#include "src/render/gl/gl_stroke2.hpp"

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
}

void GLStroke2::HandleQuadTo(const Point& from, const Point& control,
                             const Point& end) {}

void GLStroke2::HandleClose() { HandleFirstAndEndCap(); }

void GLStroke2::HandleFinish() { HandleFirstAndEndCap(); }

void GLStroke2::HandleFirstAndEndCap() {
  if (!cur_pt_.IsValid() || !first_pt_.IsValid()) {
    return;
  }

  if (*cur_pt_ == *first_pt_) {
    // TODO handle first and last join

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

  Vec2 join = center + pe;

  auto c = GetGLVertex()->AddPoint(
      center.x, center.y,
      IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 0.f, 0.f);
  auto cp1 = GetGLVertex()->AddPoint(
      p1.x, p1.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, 1.f,
      0.f);
  auto cp2 = GetGLVertex()->AddPoint(
      p2.x, p2.y, IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE, -1.f,
      0.f);

  GetGLVertex()->AddFront(c, cp1, cp2);

  // for test
  auto a = GetGLVertex()->AddPoint(p1.x, p1.y, 0.f, 0.f, 0.f);
  auto b = GetGLVertex()->AddPoint(p2.x, p2.y, 0.f, 0.f, 0.f);
  auto e = GetGLVertex()->AddPoint(join.x, join.y, 0.f, 0.f, 0.f);

  GetGLVertex()->AddFront(a, b, e);
}

void GLStroke2::HandleBevelJoinInternal(const Vec2& center, const Vec2& p1,
                                        const Vec2& p2, const Vec2& out_dir) {}

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
