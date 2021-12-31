#include "src/render/hw/hw_path_raster.hpp"

#include "src/geometry/geometry.hpp"
#include "src/render/hw/hw_mesh.hpp"

namespace skity {

void HWPathRaster::FillPath(const Path& path) {
  SetBufferType(BufferType::kStencilFront);
  stroke_ = false;

  VisitPath(path, true);

  if (path.getConvexityType() == Path::ConvexityType::kConvex) {
    SwitchStencilToColor();
  } else {
    SetBufferType(BufferType::kColor);

    Rect bounds = RasterBounds();

    FillRect(bounds);
  }
}

void HWPathRaster::StrokePath(const Path& path) {
  SetBufferType(BufferType::kStencilFront);
  stroke_ = true;

  VisitPath(path, false);

  SetBufferType(BufferType::kColor);

  Rect bounds = RasterBounds();

  FillRect(bounds);
}

void HWPathRaster::OnBeginPath() { ResetRaster(); }

void HWPathRaster::OnEndPath() {
  if (!stroke_) {
    return;
  }

  float stroke_radius = StrokeWidth() * 0.5f;
  if (curr_pt_ == first_pt_) {
    auto p2 = first_pt_ + first_pt_dir_;
    HandleLineJoin(first_pt_, p2, stroke_radius);
    return;
  }

  auto first_normal = glm::vec2{-first_pt_dir_.y, first_pt_dir_.x};

  HandleLineCap(first_pt_, first_pt_ + first_normal * stroke_radius,
                first_pt_ - first_normal * stroke_radius, -first_pt_dir_,
                stroke_radius);

  auto prev_dir = glm::normalize(curr_pt_ - prev_pt_);
  auto prev_normal = glm::vec2{-prev_dir.y, prev_dir.x};

  HandleLineCap(curr_pt_, curr_pt_ + prev_normal * stroke_radius,
                curr_pt_ - prev_normal * stroke_radius, prev_dir,
                stroke_radius);
}

void HWPathRaster::OnMoveTo(glm::vec2 const& p) {
  first_pt_ = p;
  if (stroke_) {
    return;
  }

  first_pt_index_ = AppendLineVertex(p);
}

void HWPathRaster::OnLineTo(glm::vec2 const& p1, glm::vec2 const& p2) {
  if (stroke_) {
    StrokeLineTo(p1, p2);
  } else {
    FillLineTo(p1, p2);
  }
}

void HWPathRaster::OnQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                            glm::vec2 const& p3) {
  if (stroke_) {
    StrokeQuadTo(p1, p2, p3);
  } else {
    FillQuadTo(p1, p2, p3);
  }
}

void HWPathRaster::StrokeLineTo(glm::vec2 const& p1, glm::vec2 const& p2) {
  if (p1 == first_pt_) {
    // first point
    prev_pt_ = first_pt_;
    curr_pt_ = p2;
    first_pt_dir_ = glm::normalize(p2 - p1);
  }

  float stroke_width = StrokeWidth();
  float stroke_radius = stroke_width * 0.5f;

  auto aabb = ExpandLine(p1, p2, stroke_radius);
  uint32_t a = AppendLineVertex(aabb[0]);
  uint32_t b = AppendLineVertex(aabb[1]);
  uint32_t c = AppendLineVertex(aabb[2]);
  uint32_t d = AppendLineVertex(aabb[3]);

  AppendRect(a, b, c, d);

  if (p1 != first_pt_) {
    // need to handle line join
    HandleLineJoin(p1, p2, stroke_radius);
  }

  // save prev dir
  prev_pt_ = p1;
  curr_pt_ = p2;
}

void HWPathRaster::StrokeQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                                glm::vec2 const& p3) {
  if (p1 == first_pt_) {
    // first point
    prev_pt_ = first_pt_;
    curr_pt_ = p2;
    first_pt_dir_ = glm::normalize(p2 - p1);
  }

  Orientation orientation = CalculateOrientation(p1, p2, p3);
  if (orientation == Orientation::kLinear) {
    StrokeLineTo(p1, p3);
    return;
  }

  float stroke_radius = StrokeWidth() * 0.5f;

  glm::vec2 start_dir = glm::normalize(p2 - p1);
  glm::vec2 start_vertical_line = glm::vec2{start_dir.y, -start_dir.x};
  glm::vec2 start_pt1 = p1 + start_vertical_line * stroke_radius;
  glm::vec2 start_pt2 = p1 - start_vertical_line * stroke_radius;

  glm::vec2 end_dir = glm::normalize(p3 - p2);
  glm::vec2 end_vertical_line = glm::vec2{end_dir.y, -end_dir.x};
  glm::vec2 end_pt1 = p3 + end_vertical_line * stroke_radius;
  glm::vec2 end_pt2 = p3 - end_vertical_line * stroke_radius;

  glm::vec2 control_dir = glm::normalize(start_dir + end_dir);
  glm::vec2 control_vertical_line = {control_dir.y, -control_dir.x};

  glm::vec2 outerStart = start_pt1;
  glm::vec2 outerStartControl = start_pt1 + start_dir;
  glm::vec2 outerControl;
  IntersectLineLine(outerStart, outerStartControl, end_pt1, end_pt1 - end_dir,
                    outerControl);

  glm::vec2 innerStart = start_pt2;
  glm::vec2 innerStartControl = start_pt2 + start_dir;
  glm::vec2 innerControl;
  IntersectLineLine(innerStart, innerStartControl, end_pt2, end_pt2 - end_dir,
                    innerControl);

  if (glm::length(outerControl - (outerStart + end_pt1) * 0.5f) >=
      stroke_radius * 10.f) {
    // Fixme to solve cubic stroke bad case
    prev_pt_ = p2;
    curr_pt_ = p3;
    return;
  }

  glm::vec2 control_pt1 = outerControl;
  glm::vec2 control_pt2 = innerControl;

  std::array<glm::vec2, 3> outer_quad{};
  std::array<glm::vec2, 3> inner_quad{};

  if (orientation == Orientation::kAntiClockWise) {
    outer_quad[0] = start_pt1;
    outer_quad[1] = control_pt1;
    outer_quad[2] = end_pt1;

    inner_quad[0] = start_pt2;
    inner_quad[1] = control_pt2;
    inner_quad[2] = end_pt2;
  } else {
    inner_quad[0] = start_pt1;
    inner_quad[1] = control_pt1;
    inner_quad[2] = end_pt1;

    outer_quad[0] = start_pt2;
    outer_quad[1] = control_pt2;
    outer_quad[2] = end_pt2;
  }

  AppendQuadOrSplitRecursively(outer_quad, inner_quad);

  prev_pt_ = p2;
  curr_pt_ = p3;
}

void HWPathRaster::FillLineTo(glm::vec2 const& p1, glm::vec2 const& p2) {
  if (p1 == first_pt_) {
    return;
  }

  auto orientation = CalculateOrientation(first_pt_, p1, p2);

  if (orientation == Orientation::kLinear) {
    return;
  }

  int32_t i1 = AppendLineVertex(p1);
  int32_t i2 = AppendLineVertex(p2);

  if (orientation == Orientation::kAntiClockWise) {
    AppendFrontTriangle(first_pt_index_, i1, i2);
  } else {
    AppendBackTriangle(first_pt_index_, i1, i2);
  }
}

void HWPathRaster::FillQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                              glm::vec2 const& p3) {
  if (p1 != first_pt_) {
    // need to handle outer stencil
    Orientation orientation = CalculateOrientation(first_pt_, p1, p3);
    if (orientation != Orientation::kLinear) {
      uint32_t n_start_index = AppendLineVertex(p1);
      uint32_t n_end_index = AppendLineVertex(p3);

      if (orientation == Orientation::kAntiClockWise) {
        AppendFrontTriangle(first_pt_index_, n_start_index, n_end_index);
      } else {
        AppendBackTriangle(first_pt_index_, n_start_index, n_end_index);
      }
    }
  }

  uint32_t i1 = AppendVertex(p1.x, p1.y, HW_VERTEX_TYPE_QUAD_IN, 0.f, 0.f);
  uint32_t i2 = AppendVertex(p2.x, p2.y, HW_VERTEX_TYPE_QUAD_IN, 0.5f, 0.f);
  uint32_t i3 = AppendVertex(p3.x, p3.y, HW_VERTEX_TYPE_QUAD_IN, 1.f, 1.f);

  Orientation quad_orientation = CalculateOrientation(p1, p2, p3);

  if (quad_orientation == Orientation::kAntiClockWise) {
    AppendFrontTriangle(i1, i2, i3);
  } else {
    AppendBackTriangle(i1, i2, i3);
  }
}

void HWPathRaster::HandleLineJoin(glm::vec2 const& p1, glm::vec2 const& p2,
                                  float stroke_radius) {
  Orientation orientation = CalculateOrientation(prev_pt_, p1, p2);

  auto prev_dir = glm::normalize(p1 - prev_pt_);
  auto curr_dir = glm::normalize(p2 - p1);

  auto prev_normal = glm::vec2{-prev_dir.y, prev_dir.x};
  auto current_normal = glm::vec2{-curr_dir.y, curr_dir.x};

  glm::vec2 prev_join = {};
  glm::vec2 curr_join = {};

  if (orientation == Orientation::kLinear) {
    // FIXME to solve no smooth quad line
    if (CrossProductResult(prev_pt_, p1, p2) < 0) {
      prev_join = p1 - prev_normal * stroke_radius;
      curr_join = p1 - current_normal * stroke_radius;
    } else {
      prev_join = p1 + prev_normal * stroke_radius;
      curr_join = p1 + current_normal * stroke_radius;
    }
    // FIXME: to solve parallel direction, this is caused by circle path
    if (FloatIsNan(curr_join.x) || FloatIsNan(curr_join.y)) {
      return;
    }
    HandleBevelJoinInternal(p1, prev_join, curr_join,
                            glm::normalize(prev_dir - curr_dir));
    return;
  }

  if (orientation == Orientation::kAntiClockWise) {
    prev_join = p1 - prev_normal * stroke_radius;
    curr_join = p1 - current_normal * stroke_radius;
  } else {
    prev_join = p1 + prev_normal * stroke_radius;
    curr_join = p1 + current_normal * stroke_radius;
  }

  switch (LineJoin()) {
    case Paint::kMiter_Join:
      HandleMiterJoinInternal(p1, prev_join, prev_dir, curr_join, -curr_dir);
      break;
    case Paint::kBevel_Join:
      HandleBevelJoinInternal(p1, prev_join, curr_join,
                              glm::normalize(prev_dir - curr_dir));
      break;
    case Paint::kRound_Join:
      HandleRoundJoinInternal(p1, prev_join, prev_dir, curr_join, curr_dir);
      break;
    default:
      break;
  }
}

void HWPathRaster::HandleMiterJoinInternal(Vec2 const& center, Vec2 const& p1,
                                           Vec2 const& d1, Vec2 const& p2,
                                           Vec2 const& d2) {
  Vec2 pp1 = p1 - center;
  Vec2 pp2 = p2 - center;

  Vec2 out_dir = pp1 + pp2;

  float stroke_radius = StrokeWidth() * 0.5f;

  float k = 2.f * stroke_radius * stroke_radius /
            (out_dir.x * out_dir.x + out_dir.y * out_dir.y);

  Vec2 pe = k * out_dir;

  if (glm::length(pe) >= StrokeMiter()) {
    // fallback to bevel_join
    HandleBevelJoinInternal(center, p1, p2, glm::normalize(out_dir));
    return;
  }

  Vec2 join = center + pe;

  auto c = AppendLineVertex(center);

  auto cp1 = AppendLineVertex(p1);
  auto cp2 = AppendLineVertex(p2);

  auto e = AppendLineVertex(join);

  AppendFrontTriangle(c, cp1, e);
  AppendFrontTriangle(c, cp2, e);
}

void HWPathRaster::HandleBevelJoinInternal(Vec2 const& center, Vec2 const& p1,
                                           Vec2 const& p2,
                                           Vec2 const& curr_dir) {
  auto a = AppendLineVertex(center);
  auto b = AppendLineVertex(p1);
  auto c = AppendLineVertex(p2);

  AppendFrontTriangle(a, b, c);
}

void HWPathRaster::HandleRoundJoinInternal(Vec2 const& center, Vec2 const& p1,
                                           Vec2 const& d1, Vec2 const& p2,
                                           Vec2 const& d2) {
  Vec2 out_point = center + (d1 - d2) * StrokeWidth() * FloatRoot2Over2;

  auto a = AppendCircleVertex(center, center);
  auto b = AppendCircleVertex(p1, center);
  auto c = AppendCircleVertex(p2, center);
  auto e = AppendCircleVertex(out_point, center);

  AppendFrontTriangle(a, b, e);
  AppendFrontTriangle(a, e, c);
}

void HWPathRaster::AppendQuadOrSplitRecursively(
    std::array<Vec2, 3> const& outer, std::array<Vec2, 3> const& inner) {
  if (PointInTriangle(inner[1], outer[0], outer[1], outer[2])) {
    // overlap need split
    std::array<Vec2, 3> outer_1;
    std::array<Vec2, 3> outer_2;
    std::array<Vec2, 3> inner_1;
    std::array<Vec2, 3> inner_2;
    // TODO maybe inner no need to be subdivided
    SubDividedQuad(outer.data(), outer_1.data(), outer_2.data());
    SubDividedQuad(inner.data(), inner_1.data(), inner_2.data());

    AppendQuadOrSplitRecursively(outer_1, inner_1);
    AppendQuadOrSplitRecursively(outer_2, inner_2);
  } else {
    // add quad triangle
    int32_t o_p1 =
        AppendVertex(outer[0].x, outer[0].y, HW_VERTEX_TYPE_QUAD_IN, 0.f, 0.f);
    int32_t o_p2 =
        AppendVertex(outer[1].x, outer[1].y, HW_VERTEX_TYPE_QUAD_IN, 0.5f, 0.f);
    int32_t o_p3 =
        AppendVertex(outer[2].x, outer[2].y, HW_VERTEX_TYPE_QUAD_IN, 1.f, 1.f);

    int32_t i_p1 =
        AppendVertex(inner[0].x, inner[0].y, HW_VERTEX_TYPE_QUAD_OUT, 0.f, 0.f);
    int32_t i_p2 =
        AppendVertex(inner[1].x, inner[1].y, HW_VERTEX_TYPE_QUAD_OUT, .5f, 0.f);
    int32_t i_p3 =
        AppendVertex(inner[2].x, inner[2].y, HW_VERTEX_TYPE_QUAD_OUT, 1.f, 1.f);

    AppendFrontTriangle(o_p1, o_p2, o_p3);
    AppendFrontTriangle(i_p1, i_p2, i_p3);

    // add normal triangle

    int32_t on_p1 = AppendLineVertex(outer[0]);
    int32_t on_p2 = AppendLineVertex(outer[2]);

    int32_t in_p1 = AppendLineVertex(inner[0]);
    int32_t in_p2 = AppendLineVertex(inner[1]);
    int32_t in_p3 = AppendLineVertex(inner[2]);

    AppendFrontTriangle(on_p1, in_p1, in_p2);
    AppendFrontTriangle(on_p1, in_p2, on_p2);
    AppendFrontTriangle(on_p2, in_p2, in_p3);
  }
}

}  // namespace skity