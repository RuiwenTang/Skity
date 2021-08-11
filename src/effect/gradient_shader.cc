#include "src/effect/gradient_shader.hpp"

#include <cstring>

namespace skity {

GradientShader::GradientType GradientShader::asGradient(
    GradientInfo *info) const {
  if (info) {
    CopyInfo(info);
  }
  return type_;
}

void GradientShader::CopyInfo(GradientInfo *info) const {
  info->color_count = info_.color_count;
  std::memcpy(info->point.data(), info_.point.data(),
              info_.point.size() * sizeof(Point));

  std::memcpy(info->radius.data(), info_.radius.data(),
              info_.radius.size() * sizeof(float));

  info->colors.resize(info_.color_count);
  std::memcpy(info->colors.data(), info_.colors.data(),
              info_.color_count * sizeof(Vec4));

  if (!info_.color_offsets.empty()) {
    info->color_offsets.resize(info_.color_offsets.size());
    std::memcpy(info->color_offsets.data(), info_.color_offsets.data(),
                info_.color_offsets.size() * sizeof(float));
  }
}

LinearGradientShader::LinearGradientShader(const Point *pts, const Vec4 *colors,
                                           const float *pos, int32_t count)
    : GradientShader(GradientType::kLinear) {
  GradientInfo *info = GetGradientInfo();

  // points
  info->point[0] = pts[0];
  info->point[1] = pts[1];
  // colors
  info->color_count = count;
  info->colors.resize(count);
  std::memcpy(info->colors.data(), colors, count * sizeof(Vec4));
  // pos
  if (pos) {
    info->color_offsets.resize(count);
    std::memcpy(info->color_offsets.data(), pos, count * sizeof(float));
  }
}

Rect LinearGradientShader::GetFillRect() {
  Point p1 = GetGradientInfo()->point[0];
  Point p2 = GetGradientInfo()->point[1];

  Rect rect;
  rect.setLTRB(p1.x, p1.y, p2.x, p2.y);
  rect.sort();

  return rect;
}

}  // namespace skity