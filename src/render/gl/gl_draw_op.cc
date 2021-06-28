#include "src/render/gl/gl_draw_op.hpp"

#include "src/render/gl/gl_shader.hpp"

namespace skity {

void GLDrawOp::Draw() {
  OnBeforeDraw();
  OnDraw();
  OnAfterDraw();
}

void GLDrawOp::Init() { OnInit(); }

uint32_t GLDrawOpBuilder::front_start = 0;
uint32_t GLDrawOpBuilder::front_count = 0;
uint32_t GLDrawOpBuilder::back_start = 0;
uint32_t GLDrawOpBuilder::back_count = 0;

void GLDrawOpBuilder::UpdateFrontStart(uint32_t value) { front_start = value; }

void GLDrawOpBuilder::UpdateFrontCount(uint32_t value) { front_count = value; }

void GLDrawOpBuilder::UpdateBackStart(uint32_t value) { back_start = value; }

void GLDrawOpBuilder::UpdateBackCount(uint32_t value) { back_count = value; }

}  // namespace skity
