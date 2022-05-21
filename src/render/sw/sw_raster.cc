#include "src/render/sw/sw_raster.hpp"

namespace skity {

void SWRaster::MoveTo(float x, float y) {
  int32_t pox_x = 0;
  int32_t pox_y = 0;

  if (!curr_cell_invalid) {
    RecordCell();
  }

  pox_x = sw_up_scale(x);
  pox_y = sw_up_scale(y);

  StartCell(sw_trunc(pox_x), sw_trunc(pox_y));

  this->x_ = pox_x;
  this->y_ = pox_y;
}

void SWRaster::LineTo(float x, float y) {
  RenderLine(sw_up_scale(x), sw_up_scale(y));
}

void SWRaster::Sweep() {}

void SWRaster::RenderLine(int32_t to_x, int32_t to_y) {
  int32_t ex1 = sw_trunc(this->x_);
  int32_t ex2 = sw_trunc(to_x);
  int32_t ey1 = sw_trunc(this->y_);
  int32_t ey2 = sw_trunc(to_y);

  int32_t dx = to_x - this->x_;
  int32_t dy = to_y - this->y_;

  int32_t fx1 = this->x_ - sw_sub_pixels(ex1);
  int32_t fy1 = this->y_ - sw_sub_pixels(ey1);
  int32_t fx2 = 0;
  int32_t fy2 = 0;

  if (ex1 == ex2 && ey1 == ey2) {
    // in one cell
  } else if (dy == 0) {
    // horizontal line
    ex1 = ex2;
    SetCell(ex1, ey1);
  }
}

}  // namespace skity