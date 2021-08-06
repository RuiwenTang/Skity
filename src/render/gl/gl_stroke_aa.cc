#include "src/render/gl/gl_stroke_aa.hpp"

#include <array>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLStrokeAA::GLStrokeAA(float aa_width)
    : aa_width_(aa_width), gl_vertex_(nullptr) {}

GLMeshRange GLStrokeAA::StrokePathAA(Path const& path, GLVertex* gl_vertex) {
  GLMeshRange range{};
  gl_vertex_ = gl_vertex;

  range.aa_outline_start = gl_vertex->AAOutlineCount();
  range.aa_outline_count = 0;

  std::array<Point, 4> pts{};
  Path::Iter iter{path, false};
  for (;;) {
    Path::Verb verb = iter.next(pts.data());
    switch (verb) {
      case Path::Verb::kMove:
        prev_pt_ = pts[0];
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
      case Path::Verb::kDone:
        goto DONE;
        break;
      case Path::Verb::kClose:
      default:
        break;
    }
  }

DONE:
  range.aa_outline_count = gl_vertex->AAOutlineCount() - range.aa_outline_start;
  return range;
}

void GLStrokeAA::HandleLineTo(Point const& from, Point const& to) {
  glm::vec4 curr_dir = glm::normalize(to - from);
  glm::vec4 vertical_line = glm::vec4(curr_dir.y, -curr_dir.x, 0, 0);

  Point from_1 = from + vertical_line * aa_width_;
  Point from_2 = from - vertical_line * aa_width_;
  Point to_1 = to + vertical_line * aa_width_;
  Point to_2 = to - vertical_line * aa_width_;

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

  if (prev_pt_ != from) {
    Vector prev_dir = glm::normalize(from - prev_pt_);
    Vector prev_vertical_dir = Vector{prev_dir.y, -prev_dir.x, 0, 0};
    Vector outer_dir = (prev_dir - curr_dir) * 0.5f;
    Point aa_p1;
    if (glm::dot(outer_dir, prev_vertical_dir) < 0) {
      aa_p1 = from - prev_vertical_dir * aa_width_;
    } else {
      aa_p1 = from + prev_vertical_dir * aa_width_;
    }

    Point aa_p2 = from_1;
    if (glm::dot(outer_dir, vertical_line) < 0) {
      aa_p2 = from_2;
    }

    int32_t aa_p1_index = gl_vertex_->AddPoint(
        aa_p1.x, aa_p1.y, 0.0f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);
    int32_t aa_p2_index = gl_vertex_->AddPoint(
        aa_p2.x, aa_p2.y, 0.0f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);
    int32_t aa_from_index = gl_vertex_->AddPoint(
        from.x, from.y, 1.f, GLVertex::GL_VERTEX_TYPE_AA, 0.f, 0.f);

    gl_vertex_->AddAAOutline(aa_p1_index, aa_p2_index, aa_from_index);
  }

  prev_pt_ = from;
}

void GLStrokeAA::HandleQuadTo(Point const& start, Point const& control,
                              Point const& end) {
  std::array<Point, 3> quad{start, control, end};

  AppendQuadOrSplitRecursively(quad);
}

void GLStrokeAA::HandleConicTo(Point const& start, Point const& control,
                               Point const& end, float weight) {
  std::array<Point, 5> quads;
  Conic conic{start, control, end, weight};
  conic.chopIntoQuadsPOW2(quads.data(), 1);

  quads[0] = start;
  HandleQuadTo(quads[0], quads[1], quads[2]);
  HandleQuadTo(quads[2], quads[3], quads[4]);
}
void GLStrokeAA::HandleCubicTo(Point const& start, Point const& control1,
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

void GLStrokeAA::AppendQuadOrSplitRecursively(
    std::array<Point, 3> const& quad) {
  // TODO, this may be slow, need to optimaze
  if (CalculateOrientation(quad[0], quad[1], quad[2]) != Orientation::kLinear) {
    std::array<Point, 3> quad_1{};
    std::array<Point, 3> quad_2{};
    SubDividedQuad(quad.data(), quad_1.data(), quad_2.data());

    AppendQuadOrSplitRecursively(quad_1);
    AppendQuadOrSplitRecursively(quad_2);
  } else {
    HandleLineTo(quad[0], quad[2]);
  }
}

}  // namespace skity