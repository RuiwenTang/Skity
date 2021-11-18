#include "src/render/hw/hw_path_visitor.hpp"

#include <array>

#include "src/geometry/conic.hpp"

namespace skity {

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

  this->OnMoveTo(p);
}

void HWPathVisitor::HandleLineTo(const glm::vec2& p1, const glm::vec2& p2) {
  this->OnLineTo(p1, p2);

  // update prev_dir_ and pt
  prev_dir_ = glm::normalize(p2 - p1);
}

void HWPathVisitor::HandleQuadTo(const glm::vec2& p1, const glm::vec2& p2,
                                 const glm::vec2& p3) {
  this->OnQuadTo(p1, p2, p3);
  prev_dir_ = glm::normalize(p3 - p2);
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

  prev_dir_ = glm::normalize(p3 - p2);
}

void HWPathVisitor::HandleCubicTo(glm::vec2 const& p1, glm::vec2 const& p2,
                                  glm::vec2 const& p3, glm::vec2 const& p4) {
  Point start = {p1.x, p1.y, 0.f, 1.f};
  Point control1 = {p2.x, p2.y, 0.f, 1.f};
  Point control2 = {p3.x, p3.y, 0.f, 1.f};
  Point end = {p4.x, p4.y, 0.f, 1.f};

  std::array<Point, 4> cubic{start, control1, control2, end};

  std::array<Point, 32> sub_cubic{};

  SubDividedCubic8(cubic.data(), sub_cubic.data());

  for (int i = 0; i < 8; i++) {
    std::array<skity::Point, 3> quad{};
    skity::CubicToQuadratic(sub_cubic.data() + i * 4, quad.data());
    HandleQuadTo(quad[0], quad[1], quad[2]);
  }

  prev_dir_ = glm::normalize(p4 - p3);
}

void HWPathVisitor::HandleClose() {}

}  // namespace skity