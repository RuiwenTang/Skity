#include "src/render/hw/hw_draw.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/logging.hpp"
#include "src/render/hw/hw_pipeline.hpp"
#include "src/render/hw/hw_render_target.hpp"
#include "src/render/hw/hw_texture.hpp"

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

  // global alpha
  if (global_alpha_.IsValid()) {
    pipeline_->SetGlobalAlpha(*global_alpha_);
  }

  DoStencilIfNeed();
  if (clip_stencil_) {
    DoStencilBufferMove();
  } else {
    DoColorFill();
  }
}

void HWDraw::SetPipelineColorMode(uint32_t mode) { pipeline_mode_ = mode; }

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

void HWDraw::SetClearStencilClip(bool clear) { clear_stencil_clip_ = clear; }

void HWDraw::SetTexture(HWTexture* texture) { texture_ = texture; }

void HWDraw::SetFontTexture(HWTexture* font_texture) {
  font_texture_ = font_texture;
}

void HWDraw::SetGlobalAlpha(float alpha) { global_alpha_.Set(alpha); }

void HWDraw::DoStencilIfNeed() {
  if (stencil_front_range_.count == 0 && stencil_back_range_.count == 0) {
    return;
  }

  pipeline_->DisableColorOutput();
  pipeline_->EnableStencilTest();
  pipeline_->UpdateStencilMask(0x0F);
  pipeline_->SetPipelineColorMode(HWPipelineColorMode::kStencil);
  if (has_clip_) {
    pipeline_->UpdateStencilFunc(HWStencilFunc::LESS_OR_EQUAL, 0x10, 0x1F);
  } else {
    pipeline_->UpdateStencilFunc(HWStencilFunc::ALWAYS, 0x01, 0x0F);
  }

  if (stencil_front_range_.count > 0) {
    pipeline_->UpdateStencilOp(HWStencilOp::INCR_WRAP);
    pipeline_->DrawIndex(stencil_front_range_.start,
                         stencil_front_range_.count);
  }

  if (stencil_back_range_.count > 0) {
    pipeline_->UpdateStencilOp(HWStencilOp::DECR_WRAP);
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
      pipeline_->UpdateStencilFunc(HWStencilFunc::LESS, 0x10, 0x1F);
      pipeline_->UpdateStencilMask(0x0F);
    } else {
      pipeline_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x0, 0x0F);
      pipeline_->UpdateStencilMask(0x0F);
    }
  } else {
    if (has_clip_) {
      pipeline_->EnableStencilTest();
      pipeline_->UpdateStencilOp(HWStencilOp::KEEP);
      pipeline_->UpdateStencilFunc(HWStencilFunc::EQUAL, 0x10, 0x1F);
    } else {
      pipeline_->DisableStencilTest();
    }
  }

  if (font_texture_) {
    // slot 1 is font texture
    pipeline_->BindTexture(font_texture_, 1);
    font_texture_->Bind();
  }

  if (pipeline_mode_ == kUniformColor) {
    pipeline_->SetPipelineColorMode(HWPipelineColorMode::kUniformColor);
    if (uniform_color_.IsValid()) {
      pipeline_->SetUniformColor(*uniform_color_);
    }
  } else if (pipeline_mode_ == kLinearGradient ||
             pipeline_mode_ == kRadialGradient) {
    pipeline_->SetPipelineColorMode((HWPipelineColorMode)pipeline_mode_);
    pipeline_->SetGradientCountInfo(gradient_colors_.size(),
                                    gradient_stops_.size());
    pipeline_->SetGradientBoundInfo(*gradient_bounds_);
    pipeline_->SetGradientColors(gradient_colors_);
    if (!gradient_stops_.empty()) {
      pipeline_->SetGradientPositions(gradient_stops_);
    }
  } else if (pipeline_mode_ == kImageTexture || pipeline_mode_ == kFBOTexture) {
    pipeline_->SetPipelineColorMode((HWPipelineColorMode)pipeline_mode_);
    // slot 0 is image texture
    // slot 1 is font texture
    pipeline_->BindTexture(texture_, 0);
    texture_->Bind();
    pipeline_->SetGradientBoundInfo(*gradient_bounds_);
  }

  pipeline_->DrawIndex(color_range_.start, color_range_.count);

  if (texture_) {
    texture_->UnBind();
  }
}

void HWDraw::DoStencilBufferMove() {
  pipeline_->DisableColorOutput();
  pipeline_->EnableStencilTest();
  pipeline_->UpdateStencilMask(0xFF);
  pipeline_->UpdateStencilOp(HWStencilOp::REPLACE);
  pipeline_->SetPipelineColorMode(HWPipelineColorMode::kStencil);

  auto current_matrix = pipeline_->GetModelMatrix();
  bool need_reset = current_matrix != glm::identity<glm::mat4>();

  if (need_reset) {
    pipeline_->SetModelMatrix(glm::identity<glm::mat4>());
  }

  DoStencilBufferMoveInternal();

  if (need_reset) {
    pipeline_->SetModelMatrix(current_matrix);
  }

  pipeline_->UpdateStencilMask(0x0F);
  pipeline_->UpdateStencilOp(HWStencilOp::KEEP);
  pipeline_->DisableStencilTest();
  pipeline_->EnableColorOutput();
}

void HWDraw::DoStencilBufferMoveInternal() {
  if (color_range_.count == 0) {
    return;
  }

  if (clear_stencil_clip_) {
    // clear stencil clip value
    pipeline_->UpdateStencilFunc(HWStencilFunc::ALWAYS, 0x00, 0x0F);
    pipeline_->DrawIndex(color_range_.start, color_range_.count);
  } else if (!has_clip_) {
    // mark bit 9 if lower 8 bit is not zero
    pipeline_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x10, 0x0F);

    pipeline_->DrawIndex(color_range_.start, color_range_.count);
  } else {
    // recursive clip path

    // step 1 clear all 0x10 value
    pipeline_->UpdateStencilOp(HWStencilOp::DECR_WRAP);
    pipeline_->UpdateStencilMask(0xFF);
    pipeline_->UpdateStencilFunc(HWStencilFunc::EQUAL, 0x10, 0x1F);
    pipeline_->DrawIndex(color_range_.start, color_range_.count);

    // step 2 replace all great than 0x10 to 0x10
    pipeline_->UpdateStencilOp(HWStencilOp::REPLACE);
    pipeline_->UpdateStencilMask(0x0F);
    pipeline_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x00, 0x0F);
    pipeline_->DrawIndex(color_range_.start, color_range_.count);
  }
}

PostProcessDraw::PostProcessDraw(HWRenderTarget* render_target,
                                 std::vector<std::unique_ptr<HWDraw>> draw_list,
                                 Rect const& bounds, HWPipeline* pipeline,
                                 bool has_clip, bool clip_stencil)
    : HWDraw(pipeline, has_clip, clip_stencil),
      render_target_(render_target),
      draw_list_(std::move(draw_list)),
      bounds_(bounds) {}

PostProcessDraw::PostProcessDraw(HWRenderTarget* render_target,
                                 std::unique_ptr<HWDraw> op, Rect const& bounds,
                                 HWPipeline* pipeline, bool has_clip,
                                 bool clip_stencil)
    : HWDraw(pipeline, has_clip, clip_stencil),
      render_target_(render_target),
      bounds_(bounds) {
  draw_list_.emplace_back(std::move(op));
}

PostProcessDraw::~PostProcessDraw() = default;

void PostProcessDraw::Draw() {
  SetPipelineColorMode(HWPipelineColorMode::kImageTexture);
  DrawToRenderTarget();

  DoFilter();

  DrawToCanvas();
}

void PostProcessDraw::DrawToRenderTarget() {
  render_target_->BindHBuffer();

  GetPipeline()->BindRenderTarget(render_target_);

  glm::mat4 matrix = glm::identity<glm::mat4>();

  auto mvp = glm::ortho(bounds_.left(), bounds_.right(), bounds_.bottom(),
                        bounds_.top());

  saved_mvp_ = GetPipeline()->GetMVPMatrix();

  GetPipeline()->SetViewProjectionMatrix(mvp);

  for (const auto& op : draw_list_) {
    op->SetTransformMatrix(matrix);
    op->Draw();
  }
}

void PostProcessDraw::DoFilter() {
  // do horizontal blur
  render_target_->BindVBuffer();
  SetTexture(render_target_->HColorBuffer());
  HWDraw::Draw();

  // do vertical blur
  render_target_->BindHBuffer();
  SetTexture(render_target_->VColorBuffer());
  HWDraw::Draw();

  SetPipelineColorMode(HWPipelineColorMode::kFBOTexture);
}

void PostProcessDraw::DrawToCanvas() {
  GetPipeline()->UnBindRenderTarget(render_target_);

  SetTexture(render_target_->HColorBuffer());

  GetPipeline()->SetViewProjectionMatrix(saved_mvp_);

  HWDraw::Draw();
}

}  // namespace skity