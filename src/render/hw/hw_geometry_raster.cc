#include "src/render/hw/hw_geometry_raster.hpp"

#include <algorithm>

#include "src/render/hw/hw_mesh.hpp"

namespace skity {

HWGeometryRaster::HWGeometryRaster(HWMesh* mesh, Paint const& paint, bool msaa)
    : mesh_(mesh), paint_(paint), msaa_(msaa) {}

std::array<uint32_t, 2> HWGeometryRaster::RasterLine(const glm::vec2& p0,
                                                     const glm::vec2& p1) {
  if (paint_.getStyle() == Paint::kFill_Style) {
    // single line can not do fill operation
    return {0, 0};
  }
  float stroke_width = std::min(1.f, paint_.getStrokeWidth());
  float stroke_radius = stroke_width * 0.5f;

  bool hair_line = stroke_width <= 1.f;

  // [a,b, c, d]
  auto aabb = ExpandLine(p0, p1);

  auto a_index = AppendLineVertex(aabb[0], 1.f);
  auto b_index = AppendLineVertex(aabb[1], -1.f);
  auto c_index = AppendLineVertex(aabb[2], 1.f);
  auto d_index = AppendLineVertex(aabb[3], -1.f);

  AppendRect(a_index, b_index, c_index, d_index);

  auto dir = glm::normalize(p1 - p0);
  HandleLineCap(aabb[0], aabb[1], -dir, stroke_radius);
  HandleLineCap(aabb[2], aabb[3], dir, stroke_radius);

  return FlushIndexBuffer();
}

void HWGeometryRaster::HandleLineCap(glm::vec2 const& p0, glm::vec2 const& p1,
                                     glm::vec2 const& out_dir,
                                     float stroke_radius) {
  glm::vec2 a;
  glm::vec2 b;
  switch (paint_.getStrokeCap()) {
    case Paint::kRound_Cap:
    case Paint::kButt_Cap:
      a = p0 + out_dir * stroke_radius;
      b = p1 + out_dir * stroke_radius;
      break;
    case Paint::kSquare_Cap:
      a = p0 + out_dir * 0.1f;
      b = p0 + out_dir * 0.1f;
      break;
    default:
      return;
  }

  uint32_t i1, i2, i3, i4;
  if (paint_.getStrokeCap() == Paint::kRound_Cap) {
  } else {
    if (msaa_ && paint_.getStrokeCap() == Paint::kSquare_Cap) {
      return;
    }
    i1 = AppendLineVertex(p0, 0.f);
    i2 = AppendLineVertex(p1, 0.f);

    i3 = AppendLineVertex(a, 1.f);
    i4 = AppendLineVertex(b, 1.f);
  }

  AppendRect(i1, i2, i3, i4);
}

}  // namespace skity