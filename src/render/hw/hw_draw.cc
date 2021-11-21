#include "src/render/hw/hw_draw.hpp"

#include "src/render/hw/hw_pipeline.hpp"

namespace skity {

void HWDraw::Draw() {}

void HWDraw::SetPipelineType(uint32_t type) { pipeline_type_ = type; }

void HWDraw::SetStencilRange(HWDrawRange const& front_range,
                             HWDrawRange const& back_range) {
  stencil_front_range_ = front_range;
  stencil_back_range_ = back_range;
}

void HWDraw::SetColorRange(HWDrawRange const& color_range) {
  color_range_ = color_range;
}

void HWDraw::SetUniformColor(glm::vec4 const& color) {
  uniform_color_.Set(color);
}

void HWDraw::SetTransformMatrix(glm::mat4 const& matrix) {
  transform_matrix_.Set(matrix);
}

void HWDraw::SetGradientBounds(glm::vec2 const& p0, glm::vec2 const& p1) {
  gradient_bounds_.Set(glm::vec4{p0, p1});
}

void HWDraw::SetGradientColors(std::vector<glm::vec4> const& colors) {
  gradient_colors_ = colors;
}

void HWDraw::SetGradientPositions(std::vector<float> const& pos) {
  gradient_stops_ = pos;
}

}  // namespace skity