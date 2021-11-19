#include "src/render/hw/hw_draw.hpp"

#include "src/render/hw/hw_pipeline.hpp"

namespace skity {

void HWDraw::Draw() {
  this->OnBeforeDraw(HasClip());
  this->OnDraw(HasClip());
  this->OnEndDraw(HasClip());
}

void FillColorDraw::OnBeforeDraw(bool has_clip) {
  if (stencil_discard_) {
    // stencil discard need clear fragment stencil value to prevent overdraw
    GetPipeline()->UpdateStencilOp(HWStencilOp::REPLACE);
  } else {
    GetPipeline()->UpdateStencilOp(HWStencilOp::KEEP);
  }

  if (has_clip) {
    GetPipeline()->UpdateStencilMask(0x0F, 0x1F);
    if (stencil_discard_) {
      GetPipeline()->UpdateStencilFunc(HWStencilFunc::LESS, 0x10);
    } else {
      GetPipeline()->UpdateStencilFunc(HWStencilFunc::EQUAL, 0x10);
    }
  } else {
    GetPipeline()->UpdateStencilMask(0x0F, 0x0F);
    if (stencil_discard_) {
      GetPipeline()->UpdateStencilFunc(HWStencilFunc::NOT_EQUAL, 0x00);
    } else {
      GetPipeline()->UpdateStencilFunc(HWStencilFunc::ALWAYS, 0x0);
    }
  }
}

void FillColorDraw::OnDraw(bool has_clip) {
  GetPipeline()->DrawIndex(draw_range_.start, draw_range_.count);
}

}  // namespace skity