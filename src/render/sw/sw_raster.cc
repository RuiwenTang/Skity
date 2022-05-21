#include "src/render/sw/sw_raster.hpp"

#include <algorithm>

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

void SWRaster::DoRasteration() {
  if (!curr_cell_invalid) {
    RecordCell();
  }

  spans_.clear();

  if (cells_y_.empty()) {
    return;
  }

  for (int32_t current_y = min_ey_; current_y <= max_ey_; current_y++) {
    if (cells_y_.count(current_y) == 0) {
      continue;
    }

    auto const& list = cells_y_[current_y];

    int32_t cover = 0;
    int32_t x = min_ex_;

    for (auto const& cell : list) {
      if (cell.x > x && cover != 0) {
        // inner solid
        Sweepline(x, current_y, cover * (SW_ONE_PIXEL * 2), cell.x - x);
      }

      cover += cell.cover;
      int32_t area = cover * (SW_ONE_PIXEL * 2) - cell.area;

      if (area != 0 && cell.x >= min_ex_) {
        // edge with aa alpha
        Sweepline(cell.x, current_y, area, 1);
      }

      x = cell.x + 1;
    }
  }
}

void SWRaster::StartCell(int32_t ex, int32_t ey) {
  this->curr_cel_.area = 0;
  this->curr_cel_.cover = 0;

  this->UpdateXY(ex, ey);

  this->curr_cell_invalid = false;

  SetCell(ex, ey);
}

void SWRaster::SetCell(int32_t ex, int32_t ey) {
  if (ex != this->ex_ || ey != this->ey_) {
    if (!this->curr_cell_invalid) {
      RecordCell();
    }

    this->curr_cel_.area = 0;
    this->curr_cel_.cover = 0;

    UpdateXY(ex, ey);
  }

  this->curr_cell_invalid = false;
}

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
  } else if (dx == 0) {
    // vertical line
    if (dy > 0) {
      // go up
      do {
        fy2 = SW_ONE_PIXEL;
        this->curr_cel_.cover += (fy2 - fy1);
        this->curr_cel_.area += (fy2 - fy1) * fx1 * 2;
        fy1 = 0;
        ey1++;
        SetCell(ex1, ey1);
      } while (ey1 != ey2);
    } else {
      // go down
      do {
        fy2 = 0;
        this->curr_cel_.cover += (fy2 - fy1);
        this->curr_cel_.area += (fy2 - fy1) * fx1 * 2;
        fy1 = SW_ONE_PIXEL;
        ey1--;
        SetCell(ex1, ey1);
      } while (ey1 != ey2);
    }
  } else {  // any other line

    // this determins which side line is going to
    int32_t prod = dx * fy1 - dy * fx1;
    do {
      if (prod <= 0 && prod - dx * SW_ONE_PIXEL > 0) {  // left
        fx2 = 0;
        fy2 = -1 * (0 * dx - prod) / dx;
        prod -= dy * SW_ONE_PIXEL;
        this->curr_cel_.cover += (fy2 - fy1);
        this->curr_cel_.area += (fy2 - fy1) * (fx1 + fx2);
        fx1 = SW_ONE_PIXEL;
        fy1 = fy2;
        ex1--;
      } else if (prod - dx * SW_ONE_PIXEL <= 0 &&
                 prod - dx * SW_ONE_PIXEL + dy * SW_ONE_PIXEL > 0) {  // up
        prod -= dx * SW_ONE_PIXEL;

        fx2 = -1 * prod / dy;
        fy2 = SW_ONE_PIXEL;
        this->curr_cel_.cover += (fy2 - fy1);
        this->curr_cel_.area += (fy2 - fy1) * (fx1 + fx2);
        fx1 = fx2;
        fy1 = 0;
        ey1++;
      } else if (prod - dx * SW_ONE_PIXEL + dy * SW_ONE_PIXEL <= 0 &&
                 prod + dy * SW_ONE_PIXEL >= 0) {  // right
        prod += dy * SW_ONE_PIXEL;

        fx2 = SW_ONE_PIXEL;
        fy2 = prod / dx;

        this->curr_cel_.cover += (fy2 - fy1);
        this->curr_cel_.area += (fy2 - fy1) * (fx1 + fx2);

        fx1 = 0;
        fy1 = fy2;
        ex1++;
      } else {  // down
        fx2 = (0 * dx - prod) / dy;
        fy2 = 0;

        prod += dx * SW_ONE_PIXEL;

        this->curr_cel_.cover += (fy2 - fy1);
        this->curr_cel_.area += (fy2 - fy1) * (fx1 + fx2);

        fx1 = fx2;
        fy1 = SW_ONE_PIXEL;
        ey1--;
      }

      SetCell(ex1, ey1);
    } while (ex1 != ex2 || ey1 != ey2);
  }

  fx2 = to_x - sw_sub_pixels(ex2);
  fy2 = to_y - sw_sub_pixels(ey2);

  this->curr_cel_.cover += (fy2 - fy1);
  this->curr_cel_.area += (fy2 - fy1) * (fx1 + fx2);

  this->x_ = to_x;
  this->y_ = to_y;
}

void SWRaster::RecordCell() {
  if (this->curr_cel_.area | this->curr_cel_.cover) {
    auto cell = FindCell();
    cell->area += this->curr_cel_.area;
    cell->cover += this->curr_cel_.cover;
  }
}

SWRaster::Cell* SWRaster::FindCell() {
  if (cells_y_.count(ey_) == 0) {
    std::vector<Cell> list(1);
    cells_y_[ey_] = list;

    auto cell = &cells_y_[ey_][0];
    cell->x = ex_;
    cell->area = 0;
    cell->cover = 0;

    return cell;
  }

  auto& list = cells_y_[ey_];

  int32_t index = -1;
  for (size_t i = 0; i < list.size(); i++) {
    if (list[i].x == ex_) {
      return &list[i];
    }

    if (list[i].x > ex_) {
      index = i;
      break;
    }
  }

  Cell* cell = nullptr;
  if (index != -1) {
    // insert new cell in index
    list.insert(list.begin() + index, Cell{});
    cell = &list[index];
  } else {
    list.emplace_back(Cell{});
    cell = &list.back();
  }

  cell->x = ex_;
  cell->area = 0;
  cell->cover = 0;

  return cell;
}

void SWRaster::UpdateXY(int32_t ex, int32_t ey) {
  ex_ = ex;
  ey_ = ey;

  min_ex_ = std::min(min_ex_, ex_);
  min_ey_ = std::min(min_ey_, ey_);
  max_ex_ = std::max(max_ex_, ex_);
  max_ey_ = std::max(max_ey_, ey_);
}

void SWRaster::Sweepline(int32_t x, int32_t y, int32_t area, int32_t count) {
  // coverage percentage = area / (SW_ONE_PIXEL * SW_ONE_PIXEL * 2)

  int32_t coverage = static_cast<int32_t>(area >> (SW_PIXEL_BITS * 2 + 1 - 8));

  if (coverage < 0) {
    coverage = -coverage;
  }

  if (this->even_odd_) {
    coverage &= 511;
    if (coverage > 256) {
      coverage = 512 - coverage;
    } else if (coverage == 256) {
      coverage = 255;
    }
  } else {
    if (coverage >= 256) {
      coverage = 255;
    }
  }

  x = std::min(x, std::numeric_limits<int32_t>::max());
  y = std::min(y, std::numeric_limits<int32_t>::max());

  if (coverage) {
    if (!this->spans_.empty()) {
      auto p_span = &spans_.back();
      if (p_span->y == y && p_span->x + p_span->len == x &&
          p_span->cover == coverage) {
        p_span->len = p_span->len + count;
        return;
      }
    }

    Span span;
    span.x = x;
    span.y = y;
    span.len = count;
    span.cover = coverage;

    this->spans_.emplace_back(span);
  }
}

}  // namespace skity