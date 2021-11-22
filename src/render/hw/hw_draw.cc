#include "src/render/hw/hw_draw.hpp"

#include "src/render/hw/hw_pipeline.hpp"

namespace skity {

void HWDraw::Draw() {
  // update transform matrix
  if (transform_matrix_.IsValid()) {
    pipeline_->SetModelMatrix(*transform_matrix_);
  }

  // stroke width
  if (stroke_width_.IsValid()) {
    pipeline_->SetStrokeWidth(*stroke_width_);
  }

  DoStencilIfNeed();
  if (clip_stencil_) {
    DoStencilBufferMove();
  } else {
    DoColorFill();
  }
}

void HWDraw::SetPipelineType(uint32_t type) { pipeline_type_ = type; }

void HWDraw::SetStencilRange(HWDrawRange const& front_range,
                             HWDrawRange const& back_range) {
  stencil_front_range_ = front_range;
  stencil_back_range_ = back_range;
}

void HWDraw::SetColorRange(HWDrawRange const& color_range) {
  color_range_ = color_range;
}

void HWDraw::SetStrokeWidth(float width) { stroke_width_.Set(width); }

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

void HWDraw::DoStencilIfNeed() {
  if (stencil_front_range_.count == 0 && stencil_back_range_.count == 0) {
    return;
  }

  pipeline_->DisableColorOutput();
  pipeline_->EnableStencilTest();
  pipeline_->UpdateStencilMask(0x0F, 0x0F);
  pipeline_->SetPipelineMode(HWPipelineMode::kStencil);
  pipeline_->UpdateStencilFunc(HWStencilFunc::ALWAYS, 0x01);

  if (stencil_front_range_.count > 0) {
    pipeline_->UpdateStencilOp(HWStencilOp::INC_WRAP);
    pipeline_->DrawIndex(stencil_front_range_.start,
                         stencil_front_range_.count);
  }

  if (stencil_back_range_.count > 0) {
    pipeline_->UpdateStencilOp(HWStencilOp::DESC_WRAP);
    pipeline_->DrawIndex(stencil_back_range_.start, stencil_back_range_.count);
  }

  pipeline_->EnableColorOutput();
}

void HWDraw::DoColorFill() {
  bool has_stencil_discard =
      stencil_back_range_.count != 0 || stencil_front_range_.count != 0;

  if (has_stencil_discard) {
    pipeline_->UpdateStencilOp(HWStencilOp::REPLACE);
    if (has_clip_) {
      pipeline_->UpdateStencilFunc(HWStencilFunc::LESS, 0x10);
      pipeline_->UpdateStencilMask(0x0F, 0x1F);
    } else {
      pipeline_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x0);
      pipeline_->UpdateStencilMask(0x0F, 0x0F);
    }
  } else {
    pipeline_->DisableStencilTest();
  }

  if (uniform_color_.IsValid()) {
    pipeline_->SetUniformColor(*uniform_color_);
  }

  pipeline_->DrawIndex(color_range_.start, color_range_.count);
}

void HWDraw::DoStencilBufferMove() {}

}  // namespace skity