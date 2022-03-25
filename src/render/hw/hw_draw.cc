#include "src/render/hw/hw_draw.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/logging.hpp"
#include "src/render/hw/hw_render_target.hpp"
#include "src/render/hw/hw_renderer.hpp"
#include "src/render/hw/hw_texture.hpp"

namespace skity {

void HWDraw::Draw() {
  // update transform matrix
  if (transform_matrix_.IsValid()) {
    renderer_->SetModelMatrix(*transform_matrix_);
  }

  // stroke width
  if (stroke_width_.IsValid()) {
    renderer_->SetStrokeWidth(*stroke_width_);
  }

  // global alpha
  if (global_alpha_.IsValid()) {
    renderer_->SetGlobalAlpha(*global_alpha_);
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

  renderer_->DisableColorOutput();
  renderer_->EnableStencilTest();
  renderer_->UpdateStencilMask(0x0F);
  renderer_->SetPipelineColorMode(HWPipelineColorMode::kStencil);
  if (has_clip_) {
    renderer_->UpdateStencilFunc(HWStencilFunc::LESS_OR_EQUAL, 0x10, 0x1F);
  } else {
    renderer_->UpdateStencilFunc(HWStencilFunc::ALWAYS, 0x01, 0x0F);
  }

  if (stencil_front_range_.count > 0) {
    renderer_->UpdateStencilOp(HWStencilOp::INCR_WRAP);
    renderer_->DrawIndex(stencil_front_range_.start,
                         stencil_front_range_.count);
  }

  if (stencil_back_range_.count > 0) {
    renderer_->UpdateStencilOp(HWStencilOp::DECR_WRAP);
    renderer_->DrawIndex(stencil_back_range_.start, stencil_back_range_.count);
  }

  renderer_->EnableColorOutput();
}

void HWDraw::DoColorFill() {
  HandleStencilDiscard();

  if (font_texture_) {
    // slot 1 is font texture
    renderer_->BindTexture(font_texture_, 1);
    font_texture_->Bind();
  }

  if (pipeline_mode_ == kUniformColor) {
    renderer_->SetPipelineColorMode(HWPipelineColorMode::kUniformColor);
    if (uniform_color_.IsValid()) {
      renderer_->SetUniformColor(*uniform_color_);
    }
  } else if (pipeline_mode_ == kLinearGradient ||
             pipeline_mode_ == kRadialGradient) {
    renderer_->SetPipelineColorMode((HWPipelineColorMode)pipeline_mode_);
    renderer_->SetGradientCountInfo(gradient_colors_.size(),
                                    gradient_stops_.size());
    renderer_->SetGradientBoundInfo(*gradient_bounds_);
    renderer_->SetGradientColors(gradient_colors_);
    if (!gradient_stops_.empty()) {
      renderer_->SetGradientPositions(gradient_stops_);
    }
  } else if (pipeline_mode_ >= kImageTexture ||
             pipeline_mode_ <= kInnerBlurMix) {
    renderer_->SetPipelineColorMode((HWPipelineColorMode)pipeline_mode_);

    BindTexture();

    renderer_->SetGradientBoundInfo(*gradient_bounds_);
  }

  renderer_->DrawIndex(color_range_.start, color_range_.count);

  if (texture_) {
    texture_->UnBind();
  }

  DoStencilBufferClearIfNeed();
}

void HWDraw::DoStencilBufferMove() {
  renderer_->DisableColorOutput();
  renderer_->EnableStencilTest();
  renderer_->UpdateStencilMask(0xFF);
  renderer_->UpdateStencilOp(HWStencilOp::REPLACE);
  renderer_->SetPipelineColorMode(HWPipelineColorMode::kStencil);

  auto current_matrix = renderer_->GetModelMatrix();
  bool need_reset = current_matrix != glm::identity<glm::mat4>();

  if (need_reset) {
    renderer_->SetModelMatrix(glm::identity<glm::mat4>());
  }

  DoStencilBufferMoveInternal();

  if (need_reset) {
    renderer_->SetModelMatrix(current_matrix);
  }

  renderer_->UpdateStencilMask(0x0F);
  renderer_->UpdateStencilOp(HWStencilOp::KEEP);
  renderer_->DisableStencilTest();
  renderer_->EnableColorOutput();
}

void HWDraw::DoStencilBufferMoveInternal() {
  if (color_range_.count == 0) {
    return;
  }

  if (clear_stencil_clip_) {
    // clear stencil clip value
    renderer_->UpdateStencilFunc(HWStencilFunc::ALWAYS, 0x00, 0x0F);
    renderer_->DrawIndex(color_range_.start, color_range_.count);
  } else if (!has_clip_) {
    // mark bit 9 if lower 8 bit is not zero
    renderer_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x10, 0x0F);

    renderer_->DrawIndex(color_range_.start, color_range_.count);
  } else {
    // recursive clip path

    // step 1 clear all 0x10 value
    renderer_->UpdateStencilOp(HWStencilOp::DECR_WRAP);
    renderer_->UpdateStencilMask(0xFF);
    renderer_->UpdateStencilFunc(HWStencilFunc::EQUAL, 0x10, 0x1F);
    renderer_->DrawIndex(color_range_.start, color_range_.count);

    // step 2 replace all great than 0x10 to 0x10
    renderer_->UpdateStencilOp(HWStencilOp::REPLACE);
    renderer_->UpdateStencilMask(0x0F);
    renderer_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x00, 0x0F);
    renderer_->DrawIndex(color_range_.start, color_range_.count);
  }
}

void HWDraw::DoStencilBufferClearIfNeed() {
  if (!even_odd_fill_) {
    return;
  }

  // since even-odd fill only replace part of covered stencil buffer
  // we should clear other no-event stencil buffer in this step
  // and this need another draw for color fill range
  renderer_->DisableColorOutput();
  renderer_->UpdateStencilOp(HWStencilOp::REPLACE);
  renderer_->UpdateStencilMask(0x0F);
  renderer_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x00, 0x0F);
  renderer_->DrawIndex(color_range_.start, color_range_.count);

  renderer_->EnableColorOutput();
}

void HWDraw::HandleStencilDiscard() {
  bool has_stencil_discard =
      stencil_back_range_.count != 0 || stencil_front_range_.count != 0;

  if (has_stencil_discard) {
    renderer_->UpdateStencilOp(HWStencilOp::REPLACE);
    if (even_odd_fill_) {
      HandleEvenOddStencilDiscard();
    } else {
      HandleNormalStencilDiscard();
    }
    renderer_->UpdateStencilMask(0x0F);
  } else {
    if (has_clip_) {
      renderer_->EnableStencilTest();
      renderer_->UpdateStencilOp(HWStencilOp::KEEP);
      renderer_->UpdateStencilFunc(HWStencilFunc::EQUAL, 0x10, 0x1F);
    } else {
      renderer_->DisableStencilTest();
    }
  }
}

void HWDraw::HandleNormalStencilDiscard() {
  if (has_clip_) {
    renderer_->UpdateStencilFunc(HWStencilFunc::LESS, 0x10, 0x1F);
  } else {
    renderer_->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x0, 0x0F);
  }
}

void HWDraw::HandleEvenOddStencilDiscard() {
  if (has_clip_) {
    renderer_->UpdateStencilFunc(HWStencilFunc::LESS, 0x10, 0x11);
  } else {
    renderer_->UpdateStencilFunc(HWStencilFunc::LESS, 0x00, 0x01);
  }
}

void HWDraw::BindTexture() {
  // slot 0 is image texture
  // slot 1 is font texture
  renderer_->BindTexture(texture_, 0);
  texture_->Bind();
  if (pipeline_mode_ >= kSolidBlurMix && pipeline_mode_ <= kInnerBlurMix) {
    renderer_->BindTexture(font_texture_, 1);
    font_texture_->Bind();
  }
}

PostProcessDraw::PostProcessDraw(HWRenderTarget* render_target,
                                 std::vector<std::unique_ptr<HWDraw>> draw_list,
                                 Rect const& bounds, HWRenderer* pipeline,
                                 bool has_clip, bool clip_stencil)
    : HWDraw(pipeline, has_clip, clip_stencil),
      render_target_(render_target),
      draw_list_(std::move(draw_list)),
      bounds_(bounds) {}

PostProcessDraw::PostProcessDraw(HWRenderTarget* render_target,
                                 std::unique_ptr<HWDraw> op, Rect const& bounds,
                                 HWRenderer* pipeline, bool has_clip,
                                 bool clip_stencil)
    : HWDraw(pipeline, has_clip, clip_stencil),
      render_target_(render_target),
      bounds_(bounds) {
  draw_list_.emplace_back(std::move(op));
}

PostProcessDraw::~PostProcessDraw() = default;

void PostProcessDraw::Draw() {
  SetPipelineColorMode(HWPipelineColorMode::kImageTexture);

  SaveTransform();

  DrawToRenderTarget();

  DoFilter();

  DrawToCanvas();
}

void PostProcessDraw::DrawToRenderTarget() {
  GetPipeline()->BindRenderTarget(render_target_);

  render_target_->BindColorTexture();

  glm::mat4 matrix = glm::identity<glm::mat4>();

  auto mvp = glm::ortho(bounds_.left(), bounds_.right(), bounds_.bottom(),
                        bounds_.top());

  saved_mvp_ = GetPipeline()->GetMVPMatrix();

  GetPipeline()->SetViewProjectionMatrix(mvp);

  for (const auto& op : draw_list_) {
    op->SetTransformMatrix(matrix);
    op->SetHasClip(false);
    op->Draw();
  }

  render_target_->BlitColorTexture();
}

void PostProcessDraw::DoFilter() {
  // pass blur radius through stroke width uniform
  SetStrokeWidth(blur_radius_);
  // do horizontal blur
  render_target_->BindHorizontalTexture();
  SetTexture(render_target_->ColorTexture());
  SetFontTexture(render_target_->HorizontalTexture());
  SetPipelineColorMode(HWPipelineColorMode::kHorizontalBlur);
  HWDraw::Draw();

  // do vertical blur
  render_target_->BindVerticalTexture();
  SetTexture(render_target_->HorizontalTexture());
  SetFontTexture(render_target_->VerticalTexture());
  SetPipelineColorMode(HWPipelineColorMode::kVerticalBlur);
  HWDraw::Draw();
}

void PostProcessDraw::DrawToCanvas() {
  GetPipeline()->UnBindRenderTarget(render_target_);

  RestoreTransform();

  SetTexture(render_target_->VerticalTexture());
  if (blur_style_ == BlurStyle::kNormal) {
    SetPipelineColorMode(HWPipelineColorMode::kFBOTexture);
  } else if (blur_style_ == BlurStyle::kSolid) {
    SetPipelineColorMode(HWPipelineColorMode::kSolidBlurMix);
  } else if (blur_style_ == BlurStyle::kOuter) {
    SetPipelineColorMode(HWPipelineColorMode::kOuterBlurMix);
  } else if (blur_style_ == BlurStyle::kInner) {
    SetPipelineColorMode(HWPipelineColorMode::kInnerBlurMix);
  }

  if (blur_style_ != BlurStyle::kNormal) {
    // raw color texture is pass through font texture
    SetFontTexture(render_target_->ColorTexture());
  }

  GetPipeline()->SetViewProjectionMatrix(saved_mvp_);

  HWDraw::Draw();
}

void PostProcessDraw::SaveTransform() {
  auto const& matrix = TransformMatrix();

  if (!matrix.IsValid()) {
    return;
  }

  saved_transform_ = *matrix;

  SetTransformMatrix(glm::identity<glm::mat4>());
}

void PostProcessDraw::RestoreTransform() {
  SetTransformMatrix(saved_transform_);
}

}  // namespace skity