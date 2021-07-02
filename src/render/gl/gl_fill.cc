#include "src/render/gl/gl_fill.hpp"

#include <array>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLMeshRange GLFill::fillPath(Path const& path, Paint const& paint,
                             GLVertex* gl_vertex) {
  GLMeshRange range{};
  range.front_start = gl_vertex->FrontCount();
  range.front_count = 0;
  range.back_start = gl_vertex->BackCount();
  range.back_count = 0;

  Path::Iter iter(path, true);
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
  range.front_count = gl_vertex->FrontCount() - range.front_start;
  range.back_count = gl_vertex->BackCount() - range.back_start;
  return range;
}

void GLFill::HandleMoveTo(Point const& pt) {
  start_pt_ = pt;
  start_pt_index_ = gl_vertex_->AddPoint(
      pt.x, pt.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
}

void GLFill::HandleLineTo(Point const& from, Point const& to) {
  if (start_pt_ == from) {
    // first line to
    return;
  }

  Orientation orientation = CalculateOrientation(start_pt_, from, to);
  if (orientation == Orientation::kLinear) {
    // near a line ignore
    return;
  }
  int32_t from_index = gl_vertex_->AddPoint(
      from.x, from.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
  int32_t to_index = gl_vertex_->AddPoint(
      to.x, to.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
  if (orientation == Orientation::kAntiClockWise) {
    gl_vertex_->AddFront(start_pt_index_, from_index, to_index);
  } else {
    gl_vertex_->AddBack(start_pt_index_, from_index, to_index);
  }
}

void GLFill::HandleQuadTo(Point const& start, Point const& control,
                          Point const& end) {
  if (start != start_pt_) {
    // need to handle outer stencil
    Orientation edge_orientation = CalculateOrientation(start_pt_, start, end);
    if (edge_orientation != Orientation::kLinear) {
      int32_t normal_start_index = gl_vertex_->AddPoint(
          start.x, start.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
      int32_t normal_end_index = gl_vertex_->AddPoint(
          end.x, end.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

      if (edge_orientation == Orientation::kAntiClockWise) {
        gl_vertex_->AddFront(start_pt_index_, normal_start_index,
                             normal_end_index);
      } else {
        gl_vertex_->AddBack(start_pt_index_, normal_start_index,
                            normal_end_index);
      }
    }
  }

  Orientation quad_orientation = CalculateOrientation(start, control, end);

  if (quad_orientation == Orientation::kLinear) {
    return;
  }

  int32_t start_index = gl_vertex_->AddPoint(
      start.x, start.y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.f, 0.f);
  int32_t control_index = gl_vertex_->AddPoint(
      control.x, control.y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.5f, 0.f);
  int32_t end_index = gl_vertex_->AddPoint(
      end.x, end.y, GLVertex::GL_VERTEX_TYPE_QUAD, 1.f, 1.f);

  if (quad_orientation == Orientation::kAntiClockWise) {
    gl_vertex_->AddFront(start_index, control_index, end_index);
  } else {
    gl_vertex_->AddBack(start_index, control_index, end_index);
  }
}

void GLFill::HandleConicTo(Point const& start, Point const& control,
                           Point const& end, float weight) {
  std::array<Point, 5> quads;
  Conic conic{start, control, end, weight};
  conic.chopIntoQuadsPOW2(quads.data(), 1);
  quads[0] = start;

  HandleQuadTo(quads[0], quads[1], quads[2]);
  HandleQuadTo(quads[2], quads[3], quads[4]);
}

void GLFill::HandleCubicTo(Point const& start, Point const& control1,
                           Point const& control2, Point const& end) {
  std::array<Point, 4> cubic{start, control1, control2, end};

  std::array<skity::Point, 32> sub_cubics;
  SubDividedCubic8(cubic.data(), sub_cubics.data());

  for (int i = 0; i < 8; i++) {
    std::array<skity::Point, 3> quad;
    skity::CubicToQuadratic(sub_cubics.data() + i * 4, quad.data());
    HandleQuadTo(quad[0], quad[1], quad[2]);
  }
}

void GLFill::HandleClose() {}

}  // namespace skity