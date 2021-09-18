
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

  if (orientation == Orientation::kClockWise) {
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
                                        const Vec2& d2) {}

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

}  // namespace skity
