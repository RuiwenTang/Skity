#include "src/render/hw/hw_geometry_raster.hpp"

#include <algorithm>

#include "src/geometry/math.hpp"
#include "src/render/hw/hw_mesh.hpp"

namespace skity {

HWGeometryRaster::HWGeometryRaster(HWMesh* mesh, Paint const& paint)
    : mesh_(mesh), paint_(paint) {}

void HWGeometryRaster::RasterLine(const glm::vec2& p0, const glm::vec2& p1) {
  if (paint_.getStyle() == Paint::kFill_Style) {
    // single line can not do fill operation
    return;
  }
  float stroke_width = StrokeWidth();
  float stroke_radius = stroke_width * 0.5f;

  bool hair_line = stroke_width <= 1.f;

  // [a, b, c, d]
  auto aabb = ExpandLine(p0, p1, stroke_radius);

  auto a_index = AppendLineVertex(aabb[0]);
  auto b_index = AppendLineVertex(aabb[1]);
  auto c_index = AppendLineVertex(aabb[2]);
  auto d_index = AppendLineVertex(aabb[3]);

  AppendRect(a_index, b_index, c_index, d_index);

  auto dir = glm::normalize(p1 - p0);
  HandleLineCap(p0, aabb[0], aabb[1], -dir, stroke_radius);
  HandleLineCap(p1, aabb[2], aabb[3], dir, stroke_radius);
}

void HWGeometryRaster::RasterRect(const Rect& rect) {
  if (paint_.getStyle() == Paint::kFill_Style) {
    FillRect(rect);
  } else {
    StrokeRect(rect);
  }
}

void HWGeometryRaster::FillCircle(float cx, float cy, float radius) {
  auto p1 = glm::vec2{cx - radius, cy - radius};
  auto p2 = glm::vec2{cx - radius, cy + radius};
  auto p3 = glm::vec2{cx + radius, cy - radius};
  auto p4 = glm::vec2{cx + radius, cy + radius};

  auto center = glm::vec2{cx, cy};
  auto a = AppendCircleVertex(p1, center);
  auto b = AppendCircleVertex(p2, center);
  auto c = AppendCircleVertex(p3, center);
  auto d = AppendCircleVertex(p4, center);

  AppendRect(a, b, c, d);
}

void HWGeometryRaster::ResetRaster() {}

void HWGeometryRaster::FlushRaster() {
  // stencil front
  if (!stencil_front_buffer_.empty()) {
    stencil_front_start_ = mesh_->IndexBase();
    stencil_front_count_ = stencil_front_buffer_.size();
    mesh_->AppendIndices(stencil_front_buffer_);
  }

  if (!stencil_back_buffer_.empty()) {
    stencil_back_start_ = mesh_->IndexBase();
    stencil_back_count_ = stencil_back_buffer_.size();
    mesh_->AppendIndices(stencil_back_buffer_);
  }

  if (!color_buffer_.empty()) {
    color_start_ = mesh_->IndexBase();
    color_count_ = color_buffer_.size();
    mesh_->AppendIndices(color_buffer_);
  }
}

float HWGeometryRaster::StrokeWidth() const {
  return std::max(1.f, paint_.getStrokeWidth());
}

void HWGeometryRaster::HandleLineCap(glm::vec2 const& center,
                                     glm::vec2 const& p0, glm::vec2 const& p1,
                                     glm::vec2 const& out_dir,
                                     float stroke_radius) {
  if (paint_.getStrokeCap() == Paint::kSquare_Cap) {
    return;
  }

  glm::vec2 a = p0 + out_dir * stroke_radius;
  glm::vec2 b = p1 + out_dir * stroke_radius;

  uint32_t i1, i2, i3, i4;
  if (paint_.getStrokeCap() == Paint::kRound_Cap) {
    i1 = AppendCircleVertex(p0, center);
    i2 = AppendCircleVertex(p1, center);

    i3 = AppendCircleVertex(a, center);
    i4 = AppendCircleVertex(b, center);
  } else {
    i1 = AppendLineVertex(p0);
    i2 = AppendLineVertex(p1);

    i3 = AppendLineVertex(a);
    i4 = AppendLineVertex(b);
  }

  AppendRect(i1, i2, i3, i4);
}

std::array<glm::vec2, 4> HWGeometryRaster::ExpandLine(glm::vec2 const& p0,
                                                      glm::vec2 const& p1,
                                                      float stroke_radius) {
  std::array<glm::vec2, 4> ret = {};

  glm::vec2 dir = glm::normalize(p1 - p0);
  glm::vec2 normal = {-dir.y, dir.x};

  ret[0] = p0 + normal * stroke_radius;
  ret[1] = p0 - normal * stroke_radius;

  ret[2] = p1 + normal * stroke_radius;
  ret[3] = p1 - normal * stroke_radius;

  return ret;
}

uint32_t HWGeometryRaster::AppendLineVertex(glm::vec2 const& p) {
  return mesh_->AppendVertex(p.x, p.y, HW_VERTEX_TYPE_LINE_NORMAL);
}

uint32_t HWGeometryRaster::AppendCircleVertex(glm::vec2 const& p,
                                              glm::vec2 const& center) {
  return mesh_->AppendVertex(p.x, p.y, HW_VERTEX_TYPE_CIRCLE, center.x,
                             center.y);
}

void HWGeometryRaster::AppendRect(uint32_t a, uint32_t b, uint32_t c,
                                  uint32_t d) {
  /**
   *   a --------- c
   *   |           |
   *   |           |
   *   b-----------d
   */
  auto& buffer = CurrentIndexBuffer();

  buffer.emplace_back(a);
  buffer.emplace_back(b);
  buffer.emplace_back(c);

  buffer.emplace_back(b);
  buffer.emplace_back(d);
  buffer.emplace_back(c);
}

void HWGeometryRaster::AppendFrontTriangle(uint32_t a, uint32_t b, uint32_t c) {
  stencil_front_buffer_.emplace_back(a);
  stencil_front_buffer_.emplace_back(b);
  stencil_front_buffer_.emplace_back(c);
}

void HWGeometryRaster::AppendBackTriangle(uint32_t a, uint32_t b, uint32_t c) {
  stencil_back_buffer_.emplace_back(a);
  stencil_back_buffer_.emplace_back(b);
  stencil_back_buffer_.emplace_back(c);
}

std::vector<uint32_t>& HWGeometryRaster::CurrentIndexBuffer() {
  switch (buffer_type_) {
    case kColor:
      return color_buffer_;
    case kStencilFront:
      return stencil_front_buffer_;
    case kStencilBack:
      return stencil_back_buffer_;
  }

  return color_buffer_;
}

void HWGeometryRaster::FillRect(Rect const& rect) {
  uint32_t a = AppendLineVertex(glm::vec2{rect.left(), rect.top()});
  uint32_t b = AppendLineVertex(glm::vec2{rect.left(), rect.bottom()});
  uint32_t c = AppendLineVertex(glm::vec2{rect.right(), rect.top()});
  uint32_t d = AppendLineVertex(glm::vec2{rect.right(), rect.bottom()});

  AppendRect(a, b, c, d);
}

void HWGeometryRaster::StrokeRect(Rect const& rect) {
  float stroke_width = StrokeWidth();
  float expand_length = stroke_width * 0.5f;

  auto p1 = glm::vec2{rect.left(), rect.top()};
  auto p2 = glm::vec2{rect.right(), rect.top()};
  auto p3 = glm::vec2{rect.right(), rect.bottom()};
  auto p4 = glm::vec2{rect.left(), rect.bottom()};

  auto dir_p12 = glm::normalize(p2 - p1);
  auto dir_p23 = glm::normalize(p3 - p2);
  auto dir_p34 = glm::normalize(p4 - p3);
  auto dir_p41 = glm::normalize(p1 - p4);

  auto out_dir_p1 = dir_p41 - dir_p12;
  auto out_dir_p2 = dir_p12 - dir_p23;
  auto out_dir_p3 = dir_p23 - dir_p34;
  auto out_dir_p4 = dir_p34 - dir_p41;

  auto p1_o = p1 + out_dir_p1 * expand_length;
  auto p1_i = p1 - out_dir_p1 * expand_length;

  auto p2_o = p2 + out_dir_p2 * expand_length;
  auto p2_i = p2 - out_dir_p2 * expand_length;

  auto p3_o = p3 + out_dir_p3 * expand_length;
  auto p3_i = p3 - out_dir_p3 * expand_length;

  auto p4_o = p4 + out_dir_p4 * expand_length;
  auto p4_i = p4 - out_dir_p4 * expand_length;

  auto p1_o_index = AppendLineVertex(p1_o);
  auto p1_i_index = AppendLineVertex(p1_i);
  auto p2_o_index = AppendLineVertex(p2_o);
  auto p2_i_index = AppendLineVertex(p2_i);
  auto p3_o_index = AppendLineVertex(p3_o);
  auto p3_i_index = AppendLineVertex(p3_i);
  auto p4_o_index = AppendLineVertex(p4_o);
  auto p4_i_index = AppendLineVertex(p4_i);

  AppendRect(p1_o_index, p1_i_index, p2_o_index, p2_i_index);
  AppendRect(p2_o_index, p2_i_index, p3_o_index, p3_i_index);
  AppendRect(p3_o_index, p3_i_index, p4_o_index, p4_i_index);
  AppendRect(p4_o_index, p4_i_index, p1_o_index, p1_i_index);
}

}  // namespace skity