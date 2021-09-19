
#include "src/render/gl/gl_stroke2.hpp"

#include "src/geometry/geometry.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLStroke2::GLStroke2(const Paint& paint, GLVertex2* gl_vertex)
    : GLPathVisitor(paint, gl_vertex),
      stroke_radius_(paint.getStrokeWidth() * 0.5f) {}

void GLStroke2::HandleMoveTo(const Point& pt) { first_pt_.Set(Vec2{pt}); }

void GLStroke2::HandleLineTo(const Point& from, const Point& to) {
  Vec2 from_vec2 = Vec2{from};
  Vec2 to_vec2 = Vec2{to};

  if (first_pt_.IsValid() && *first_pt_ == from_vec2) {
    // first line to calculate prev_join_1 and prev_join_2
    Vec2 dir = glm::normalize(to - from);
    Vec2 normal = Vec2{-dir.y, dir.x};
    Vec2 prev_p1 = from_vec2 + normal * stroke_radius_;
    Vec2 prev_p2 = from_vec2 - normal * stroke_radius_;

    prev_join_1_.Set(prev_p1);
    prev_join_2_.Set(prev_p2);
  }

  PathCMD cmd{};
  cmd.p1 = from_vec2;
  cmd.p2 = to_vec2;
  cmd.verb = Path::Verb::kLine;

  HandlePrevPathCMD(cmd);

  prev_cmd_.Set(cmd);
}

void GLStroke2::HandleQuadTo(const Point& from, const Point& control,
                             const Point& end) {
  Vec2 from_vec2 = Vec2{from};
  Vec2 control_vec2 = Vec2{control};
  Vec2 end_vec2 = Vec2{end};

  PathCMD cmd{};

  cmd.p1 = from_vec2;
  cmd.p2 = control_vec2;
  cmd.p3 = end_vec2;
  cmd.verb = Path::Verb::kQuad;

  HandlePrevPathCMD(cmd);
  prev_cmd_.Set(cmd);
}

void GLStroke2::HandleClose() { HandleFirstAndEndCap(); }

void GLStroke2::HandleFinish() { HandleFirstAndEndCap(); }

void GLStroke2::HandleFirstAndEndCap() {
  if (!first_pt_.IsValid() || !prev_cmd_.IsValid()) {
    return;
  }

  HandlePrevPathCMDEnd();

  Vec2 curr_dir = GetPrevEndDirection();
  Vec2 cur_pt = GetPrevEndPoint();

  switch (GetCap()) {
    case Paint::kSquare_Cap:
      HandleSquareCapInternal(*first_pt_, *first_dir_);
      HandleSquareCapInternal(cur_pt, -curr_dir);
      break;
    case Paint::kButt_Cap:
      HandleButtCapInternal(*first_pt_, *first_dir_);
      HandleButtCapInternal(cur_pt, -curr_dir);
      break;
    case Paint::kRound_Cap:
      HandleRoundCapInternal(*first_pt_, *first_dir_);
      HandleRoundCapInternal(cur_pt, -curr_dir);
      break;
    default:
      break;
  }
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

Vec2 GLStroke2::GetPrevStartDirection() {
  if (!prev_cmd_.IsValid()) {
    return Vec2{0, 0};
  }

  if (prev_cmd_->verb == Path::Verb::kLine) {
    return glm::normalize(prev_cmd_->p2 - prev_cmd_->p1);
  }

  if (prev_cmd_->verb == Path::Verb::kQuad) {
    return glm::normalize(prev_cmd_->p2 - prev_cmd_->p1);
  }

  return Vec2{0, 0};
}

Vec2 GLStroke2::GetPrevEndDirection() {
  if (!prev_cmd_.IsValid()) {
    return Vec2{0, 0};
  }

  if (prev_cmd_->verb == Path::Verb::kLine) {
    return glm::normalize(prev_cmd_->p2 - prev_cmd_->p1);
  }

  if (prev_cmd_->verb == Path::Verb::kQuad) {
    return glm::normalize(prev_cmd_->p3 - prev_cmd_->p2);
  }

  return Vec2{0, 0};
}

Vec2 GLStroke2::GetPrevStartPoint() {
  if (!prev_cmd_.IsValid()) {
    return Vec2{0, 0};
  }

  return prev_cmd_->p1;
}

Vec2 GLStroke2::GetPrevEndPoint() {
  if (!prev_cmd_.IsValid()) {
    return Vec2{0, 0};
  }

  if (prev_cmd_->verb == Path::Verb::kQuad) {
    return prev_cmd_->p3;
  }

  return prev_cmd_->p2;
}

void GLStroke2::HandlePrevPathCMD(PathCMD const& curr_path_cmd) {
  if (!prev_cmd_.IsValid()) {
    // update first_dir
    first_dir_.Set(glm::normalize(curr_path_cmd.p2 - curr_path_cmd.p1));
    return;
  }
  const auto& cmd = *prev_cmd_;

  if (cmd.verb == Path::Verb::kLine) {
    HandleLineToInternal(curr_path_cmd);
  } else if (cmd.verb == Path::Verb::kQuad) {
  }
}

void GLStroke2::HandlePrevPathCMDEnd() {
  if (prev_cmd_->verb == Path::Verb::kLine) {
    Vec2 p1 = *prev_join_1_;
    Vec2 p2 = *prev_join_2_;

    Vec2 end = GetPrevEndPoint();
    Vec2 dir = GetPrevEndDirection();
    Vec2 normal = Vec2{-dir.y, dir.x};

    GenerateLineSquare(p1, p2, end + normal * stroke_radius_,
                       end - normal * stroke_radius_);
  }
}

std::pair<Vec2, Vec2> GLStroke2::CalculateLineJoinPoints(
    PathCMD const& curr_path_cmd) {
  Vec2 center = GetPrevEndPoint();
  Vec2 prev_dir = GetPrevEndDirection();
  Vec2 prev_normal = Vec2{-prev_dir.y, prev_dir.x};

  Vec2 curr_dir = glm::normalize(curr_path_cmd.p2 - curr_path_cmd.p1);
  Vec2 curr_normal = Vec2{-curr_dir.y, curr_dir.x};

  Vec2 p1 = center + prev_normal * stroke_radius_;
  Vec2 p2 = center + curr_normal * stroke_radius_;

  Vec2 out_dir = (curr_normal + prev_normal) * stroke_radius_;

  float k = 2.f * stroke_radius_ * stroke_radius_ /
            (out_dir.x * out_dir.x + out_dir.y * out_dir.y);

  Vec2 pe = k * out_dir;

  Vec2 join1 = center + pe;
  Vec2 join2 = center - pe;

  return std::make_pair(join1, join2);
}

void GLStroke2::HandleLineToInternal(PathCMD const& curr_path_cmd) {
  PathCMD const& prev_cmd = *prev_cmd_;

  Orientation orientation =
      CalculateOrientation(prev_cmd.p1, prev_cmd.p2, curr_path_cmd.p2);

  if (orientation == Orientation::kLinear) {
    // just add four points and rect these points
    return;
  }

  Vec2 prev_dir = GetPrevEndDirection();
  Vec2 prev_normal = Vec2{-prev_dir.y, prev_dir.x};

  Vec2 prev_to_p1 = prev_cmd.p2 + prev_normal * stroke_radius_;
  Vec2 prev_to_p2 = prev_cmd.p2 - prev_normal * stroke_radius_;

  auto join_points = CalculateLineJoinPoints(curr_path_cmd);
  // need to handle line join
  Paint::Join line_join_type = GetJoin();
  if (line_join_type == Paint::kMiter_Join &&
      glm::length(std::get<0>(join_points) - prev_cmd.p2) >= GetMiterLimit()) {
    line_join_type = Paint::kBevel_Join;
  }

  if (line_join_type == Paint::kMiter_Join) {
    Vec2 p1 = *prev_join_1_;
    Vec2 p2 = *prev_join_2_;
    Vec2 p3 = std::get<0>(join_points);
    Vec2 p4 = std::get<1>(join_points);

    GenerateLineSquare(p1, p2, p3, p4);
    prev_join_1_.Set(p3);
    prev_join_2_.Set(p4);
  }
}

void GLStroke2::GenerateLineSquare(Vec2 const& p1, Vec2 const& p2,
                                   Vec2 const& p3, Vec2 const& p4) {
  auto type = IsAntiAlias() ? GLVertex2::LINE_EDGE : GLVertex2::NONE;

  auto a = GetGLVertex()->AddPoint(p1.x, p1.y, type, 1.f, 0.f);
  auto b = GetGLVertex()->AddPoint(p2.x, p2.y, type, -1.f, 0.f);
  auto c = GetGLVertex()->AddPoint(p3.x, p3.y, type, 1.f, 0.f);
  auto d = GetGLVertex()->AddPoint(p4.x, p4.y, type, -1.f, 0.f);

  GetGLVertex()->AddFront(a, b, c);
  GetGLVertex()->AddFront(b, d, c);
}

}  // namespace skity
