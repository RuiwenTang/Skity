#include "src/render/gl/gl_path_visitor.hpp"

#include <array>

#include "src/geometry/conic.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLPathVisitor::GLPathVisitor(Paint const& paint, GLVertex2* gl_vertex)
    : gl_vertex_(gl_vertex),
      anti_alias_(paint.isAntiAlias()),
      style_(paint.getStyle()),
      join_(paint.getStrokeJoin()),
      cap_(paint.getStrokeCap()),
      stroke_width_(paint.getStrokeWidth()),
      miter_limit_(paint.getStrokeMiter()) {}

GLMeshRange GLPathVisitor::VisitPath(const Path& path, bool force_close) {
  GLMeshRange range{};

  range.front_start = GetGLVertex()->FrontCount();
  range.front_count = 0;
  range.back_start = GetGLVertex()->BackCount();
  range.back_count = 0;
  range.aa_outline_start = GetGLVertex()->AACount();
  range.aa_outline_count = 0;

  Path::Iter iter{path, force_close};
  std::array<Point, 4> pts{};

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
        HandleConicToInternal(pts[0], pts[1], pts[2], iter.conicWeight());
        break;
      case Path::Verb::kCubic:
        HandleCubicToInternal(pts[0], pts[1], pts[2], pts[3]);
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
  HandleFinish();

  range.front_count = gl_vertex_->FrontCount() - range.front_start;
  range.back_count = gl_vertex_->BackCount() - range.back_start;
  range.aa_outline_count = gl_vertex_->AACount() - range.aa_outline_start;

  return range;
}

void GLPathVisitor::HandleConicToInternal(const Point& start,
                                          const Point& control,
                                          const Point& end, float weight) {
  std::array<Point, 5> quads{};
  Conic conic{start, control, end, weight};
  conic.chopIntoQuadsPOW2(quads.data(), 1);
  quads[0] = start;

  Paint::Join save_join = join_;
  join_ = Paint::kRound_Join;
  HandleQuadTo(quads[0], quads[1], quads[2]);
  HandleQuadTo(quads[2], quads[3], quads[4]);
  join_ = save_join;
}

void GLPathVisitor::HandleCubicToInternal(const Point& start,
                                          const Point& control1,
                                          const Point& control2,
                                          const Point& end) {
  std::array<Point, 4> cubic{start, control1, control2, end};

  std::array<Point, 32> sub_cubic{};
  SubDividedCubic8(cubic.data(), sub_cubic.data());

  Paint::Join save_join = join_;
  join_ = Paint::kRound_Join;
  for (int i = 0; i < 8; i++) {
    std::array<skity::Point, 3> quad{};
    skity::CubicToQuadratic(sub_cubic.data() + i * 4, quad.data());
    HandleQuadTo(quad[0], quad[1], quad[2]);
  }
  join_ = save_join;
}

}  // namespace skity
