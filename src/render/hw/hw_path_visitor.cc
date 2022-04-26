#include "src/render/hw/hw_path_visitor.hpp"

#include <array>

#include "src/geometry/conic.hpp"

namespace skity {

#define QUAD_BEVEL_LIMIT 0.01f

static void split_cubic(glm::vec2* base) {
  glm::vec2 a, b, c;

  base[6] = base[3];
  a = base[0] + base[1];
  b = base[1] + base[2];
  c = base[2] + base[3];

  base[5] = c / 2.f;
  c += b;
  base[4] = c / 4.f;
  base[1] = a / 2.f;
  a += b;
  base[2] = a / 4.f;
  base[3] = (a + c) / 8.f;
}

static void split_quad(glm::vec2* base) {
  glm::vec2 a, b;

  base[4] = base[2];
  a = base[0] + base[1];
  b = base[1] + base[2];
  base[3] = b / 2.f;
  base[2] = (a + b) / 4.f;
  base[1] = a / 2.f;
}

void HWPathVisitor::VisitPath(Path const& path, bool force_close) {
  Path::Iter iter{path, force_close};
  std::array<Point, 4> pts = {};

  this->OnBeginPath();

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
  OnEndPath();
}

void HWPathVisitor::HandleMoveTo(const glm::vec2& p) {
  first_pt_ = p;
  prev_pt_ = p;
  this->OnMoveTo(p);
}

void HWPathVisitor::HandleLineTo(const glm::vec2& p1, const glm::vec2& p2) {
  this->OnLineTo(p1, p2);

  // update prev_dir_ and pt
  prev_dir_ = glm::normalize(p2 - p1);
  prev_pt_ = p2;
}

void HWPathVisitor::HandleQuadTo(const glm::vec2& p1, const glm::vec2& p2,
                                 const glm::vec2& p3) {
  auto saved_join = LineJoin();
  ChangeLineJoin(Paint::kRound_Join);

  std::array<glm::vec2, 33 * 3 + 1> bez_stack{};
  std::array<int32_t, 33> level_stack{};

  int32_t top, level;
  int32_t* levels = level_stack.data();

  auto arc = bez_stack.data();
  arc[0] = p3;
  arc[1] = p2;
  arc[2] = p1;

  top = 0;

  glm::vec2 delta = glm::abs(arc[2] + arc[0] - 2.f * arc[1]);

  // If delta.x or delta.y is zero maybe this is a circle approximate
  float d = 0.f;
  if (FloatNearlyZero(delta.x)) {
    d = delta.y;
  } else if (FloatNearlyZero(delta.y)) {
    d = delta.x;
  } else {
    d = glm::min(delta.x, delta.y);
  }

  if (d < QUAD_BEVEL_LIMIT) {
    goto Draw;
  }

  level = 0;
  do {
    d = d / 4.f;
    level++;
  } while (d > QUAD_BEVEL_LIMIT);

  levels[0] = level;
  do {
    level = levels[top];
    if (level > 0) {
      split_quad(arc);
      arc += 2;
      top++;
      levels[top] = levels[top - 1] = level - 1;
      continue;
    }
  Draw:
    HandleLineTo(prev_pt_, arc[0]);
    top--;
    arc -= 2;
  } while (top >= 0);

  ChangeLineJoin(saved_join);
}

void HWPathVisitor::HandleConicTo(glm::vec2 const& p1, glm::vec2 const& p2,
                                  glm::vec2 const& p3, float weight) {
  Point start = {p1, 0.f, 1.f};
  Point control = {p2.x, p2.y, 0.f, 1.f};
  Point end = {p3.x, p3.y, 0.f, 1.f};

  std::array<Point, 5> quads{};
  Conic conic{start, control, end, weight};
  conic.chopIntoQuadsPOW2(quads.data(), 1);
  quads[0] = start;

  HandleQuadTo(quads[0], quads[1], quads[2]);
  HandleQuadTo(quads[2], quads[3], quads[4]);
}

void HWPathVisitor::HandleCubicTo(glm::vec2 const& p1, glm::vec2 const& p2,
                                  glm::vec2 const& p3, glm::vec2 const& p4) {
  std::array<glm::vec2, 32 * 3 + 1> bez_stack;

  auto arc = bez_stack.data();

  arc[0] = p4;
  arc[1] = p3;
  arc[2] = p2;
  arc[3] = p1;

  glm::vec2 from = p1;
  for (;;) {
    auto delta1 = glm::abs(2.f * arc[0] - 3.f * arc[1] + arc[3]);
    auto delta2 = glm::abs(arc[0] - 3.f * arc[2] + 2.f * arc[3]);

    if (std::min(delta1.x, delta1.y) > 0.5f ||
        std::min(delta2.x, delta2.y) > 0.5f) {
      goto SPLIT;
    }

    HandleLineTo(prev_pt_, arc[0]);
    if (arc == bez_stack.data()) {
      return;
    }

    arc -= 3;
    from = arc[0];
    continue;

  SPLIT:
    split_cubic(arc);
    arc += 3;
  }
}

void HWPathVisitor::HandleClose() {}

}  // namespace skity