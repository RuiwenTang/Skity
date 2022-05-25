#include "src/render/sw/sw_path_stroke.hpp"

#include "src/geometry/geometry.hpp"

namespace skity {

void SWPathStroke::StrokePath(const Path& src, const Paint& paint) {
  stroke_width_ = paint.getStrokeWidth();
  cap_ = paint.getStrokeCap();
  join_ = paint.getStrokeJoin();
  stroke_miter_limit_ = paint.getStrokeMiter();

  Path::Iter iter{src, false};
  std::array<Point, 4> pts = {};

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
      case Path::Verb::kCubic:
        HandleCubicTo(pts[0], pts[1], pts[2], pts[3]);
        break;
      case Path::Verb::kConic:
        HandleConicTo(pts[0], pts[1], pts[2], iter.conicWeight());
        break;
      case Path::Verb::kClose:
        break;
      case Path::Verb::kDone:
        goto DONE;
        break;
    }
  }

DONE:
  HandleEnd();
  return;
}

float SWPathStroke::StrokeWidth() const { return glm::max(1.f, stroke_width_); }

void SWPathStroke::HandleMoveTo(glm::vec2 const& pt) { first_pt_ = pt; }

void SWPathStroke::HandleLineTo(glm::vec2 const& p1, glm::vec2 const& p2) {
  bool first_line = false;

  if (first_pt_ == p1) {
    first_dir_ = glm::normalize(p2 - p1);

    prev_pt_ = first_pt_;

    first_line = true;
  }

  bool teeny_line = glm::length(p2 - p1) <= 0.1f;

  if (teeny_line && cap_ == Paint::kButt_Cap) {
    return;
  }

  float stroke_radius = StrokeWidth() * 0.5f;

  glm::vec2 dir = glm::normalize(p2 - p1);
  glm::vec2 normal = glm::vec2{-dir.y, dir.x};

  auto p1_0 = p1 + normal * stroke_radius;
  auto p1_1 = p1 - normal * stroke_radius;

  auto p2_0 = p2 + normal * stroke_radius;
  auto p2_1 = p2 - normal * stroke_radius;

  if (first_line) {
    outer_.moveTo(p1_0.x, p1_0.y);
    inner_.moveTo(p1_1.x, p1_1.y);
  }

  outer_.lineTo(p2_0.x, p2_0.y);
  inner_.lineTo(p2_1.x, p2_1.y);

  // handle line join
  if (!first_line) {
    HandleLineJoin(p1, p2);
  }

  prev_pt_ = p1;
  prev_dir_ = dir;
  curr_pt_ = p2;
  curr_pt1_ = p2_0;
  curr_pt2_ = p2_1;

  if (first_line) {
    first_p1_ = p1_0;
    first_p2_ = p1_1;
  }
}

void SWPathStroke::HandleQuadTo(glm::vec2 const& p1, glm::vec2 const& control,
                                glm::vec2 const& p2) {}

void SWPathStroke::HandleCubicTo(glm::vec2 const& p1, glm::vec2 const& control1,
                                 glm::vec2 const& control2,
                                 glm::vec2 const& p2) {}

void SWPathStroke::HandleConicTo(glm::vec2 const& p1, glm::vec2 const& control,
                                 glm::vec2 const& p2, float weight) {}

void SWPathStroke::HandleLineJoin(glm::vec2 const& p1, glm::vec2 const& p2) {
  Orientation orientation = CalculateOrientation(prev_pt_, p1, p2);

  auto curr_dir = glm::normalize(p2 - p1);

  auto prev_normal = glm::vec2(-prev_dir_.y, prev_dir_.x);
  auto curr_normal = glm::vec2(-curr_dir.y, curr_dir.x);

  float stroke_radius = StrokeWidth() * 0.5f;

  glm::vec2 prev_join = {};
  glm::vec2 curr_join = {};

  glm::vec2 inner_p1 = {};
  glm::vec2 inner_p2 = {};

  if (orientation == Orientation::kLinear) {
    return;
  }

  Path* p_outer = nullptr;
  Path* p_inner = nullptr;

  if (orientation == Orientation::kAntiClockWise) {
    prev_join = p1 - prev_normal * stroke_radius;
    curr_join = p1 - curr_normal * stroke_radius;

    p_outer = &inner_;
    p_inner = &outer_;
  } else {
    prev_join = p1 + prev_normal * stroke_radius;
    curr_join = p1 + prev_normal * stroke_radius;

    p_outer = &outer_;
    p_inner = &inner_;
  }

  float detail = glm::length(prev_join - curr_join);

  switch (join_) {
    case Paint::kMiter_Join:
      HandleMiterJoinInternal(p_outer, p1, prev_join, prev_dir_, curr_join,
                              -curr_dir);
      break;
    case Paint::kBevel_Join:
      HandleBevelJoinInternal(p_outer, p1, prev_join, curr_join);
      break;
    case Paint::kRound_Join:
      if (detail < 1.f) {
        HandleBevelJoinInternal(p_outer, p1, prev_join, curr_join);
      } else {
        HandleRoundJoinInternal(p_outer, p1, prev_join, prev_dir_, curr_join,
                                curr_dir);
      }
      break;
  }

  HandleInnerJoin(p_inner, p1, inner_p1, inner_p2);
}

void SWPathStroke::HandleInnerJoin(Path* path, Vec2 const& center,
                                   Vec2 const& p1, Vec2 const& p2) {
  // using 2d geometry meaning to calculate intersection point
  auto pa = p1 - center;
  auto pb = p2 - center;
  auto pc = pa + pb;

  auto pe = pb * pb * (pa + pb) * 2.f / (pc * pc);

  auto e = center + pe;

  Point last_pt;

  if (!path->getLastPt(&last_pt)) {
    // some thing is wrong
    // this can not happen
    return;
  }

  path->setLastPt(e.x, e.y);
}

void SWPathStroke::HandleMiterJoinInternal(Path* path, Vec2 const& center,
                                           Vec2 const& p1, Vec2 const& d1,
                                           Vec2 const& p2, Vec2 const& d2) {
  Vec2 pp1 = p1 - center;
  Vec2 pp2 = p2 - center;

  Vec2 out_dir = pp1 + pp2;

  float stroke_radius = StrokeWidth() * 0.5f;

  float k = 2.f * stroke_radius * stroke_radius /
            (out_dir.x * out_dir.x + out_dir.y * out_dir.y);

  Vec2 pe = k * out_dir;

  if (glm::length(p2) >= stroke_miter_limit_) {
    // fallback to bevel_join
    HandleBevelJoinInternal(path, center, p1, p2);

    return;
  }

  Vec2 join = center + p2;

  path->lineTo(join.x, join.y);

  path->lineTo(p2.x, p2.y);
}

void SWPathStroke::HandleBevelJoinInternal(Path* path, Vec2 const& center,
                                           Vec2 const& p1, Vec2 const& p2) {
  path->lineTo(p2.x, p2.y);
}

void SWPathStroke::HandleRoundJoinInternal(Path* path, Vec2 const& center,
                                           Vec2 const& p1, Vec2 const& d1,
                                           Vec2 const& p2, Vec2 const& d2) {
  float step = 1 / 15.f;

  auto d = glm::normalize(p2 - p1);
  float stroke_radius = StrokeWidth() * 0.5f;

  for (int32_t i = 1; i < 16; i++) {
    float u = i * step;

    auto q1 = glm::mix(p1, p2, u);

    auto p = center + glm::normalize(q1 - center) * stroke_radius;

    path->lineTo(p.x, p.y);
  }
}

void SWPathStroke::HandleButtCap(Path* path, Vec2 const& p1, Vec2 const& p2) {
  Point last{};
  path->getLastPt(&last);

  Vec2 last2 = last;

  if (last2 != p1) {
    // this can not happen
    return;
  }

  path->lineTo(p2.x, p2.y);
}

void SWPathStroke::HandleSquareCap(Path* path, Vec2 const& p1, Vec2 const& p2,
                                   Vec2 const& out_dir) {
  Point last{};
  path->getLastPt(&last);

  Vec2 last2 = last;

  if (last2 != p1) {
    // this can not happen
    return;
  }

  float stroke_radius = StrokeWidth() * 0.5f;

  Vec2 p1_o = p1 + out_dir * stroke_radius;
  Vec2 p2_o = p2 + out_dir * stroke_radius;

  path->lineTo(p1_o.x, p1_o.y);
  path->lineTo(p2_o.x, p2_o.y);
  path->lineTo(p2.x, p2.y);
}

void SWPathStroke::HandleRoundCap(Path* path, Vec2 const& center,
                                  Vec2 const& p1, Vec2 const& p2,
                                  Vec2 const& out_dir) {
  Point last{};
  path->getLastPt(&last);

  Vec2 last2 = last;

  if (last2 != p1) {
    // this can not happen
    return;
  }

  float stroke_radius = StrokeWidth() * 0.5f;

  Vec2 c_o = center + out_dir * stroke_radius;

  float step = 1 / 15.f;

  auto d = glm::normalize(c_o - p1);
  for (int32_t i = 1; i < 16; i++) {
    float u = i * step;

    auto q1 = glm::mix(p1, c_o, u);

    auto p = center + glm::normalize(q1 - center) * stroke_radius;

    path->lineTo(p.x, p.y);
  }

  d = glm::normalize(p2 - c_o);
  for (int32_t i = 1; i < 16; i++) {
    float u = i * step;

    auto q1 = glm::mix(c_o, p2, u);

    auto p = center + glm::normalize(q1 - center) * stroke_radius;

    path->lineTo(p.x, p.y);
  }
}

void SWPathStroke::HandleLineCap(Path* path, Vec2 const& center, Vec2 const& p1,
                                 Vec2 const& p2, Vec2 const& out_dir) {
  switch (cap_) {
    case Paint::kButt_Cap:
      HandleButtCap(path, p1, p2);
      break;
    case Paint::kSquare_Cap:
      HandleSquareCap(path, p1, p2, out_dir);
      break;
    case Paint::kRound_Cap:
      HandleRoundCap(path, center, p1, p2, out_dir);
      break;
  }
}

void SWPathStroke::HandleEnd() {
  if (curr_pt_ == first_pt_) {
    HandleLineJoin(first_pt_, first_pt_ + first_dir_);
    return;
  }

  HandleLineCap(&outer_, curr_pt_, curr_pt1_, curr_pt2_, prev_dir_);

  HandleLineCap(&inner_, first_pt_, first_p2_, first_p1_, -first_dir_);

  outer_.reversePathTo(inner_);
}

}  // namespace skity