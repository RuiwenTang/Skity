#ifndef SKITY_SRC_RENDER_SW_SW_RASTER_HPP
#define SKITY_SRC_RENDER_SW_SW_RASTER_HPP

#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <unordered_map>
#include <vector>

#include "src/render/sw/sw_subpixel.hpp"

namespace skity {

class SWRaster final {
  struct Cell {
    int32_t x;
    int32_t cover;
    int32_t area;
  };

 public:
  SWRaster() = default;
  ~SWRaster() = default;

  void MoveTo(float x, float y);

  void LineTo(float x, float y);

  void Sweep();

 private:
  void StartCell(int32_t ex, int32_t ey);

  void SetCell(int32_t ex, int32_t ey);

  void RenderLine(int32_t to_x, int32_t to_y);

  void RecordCell();

  Cell* FindCell();

  void UpdateXY(int32_t ex, int32_t ey);

  void Sweepline(int32_t x, int32_t y, int32_t area, int32_t count);

 private:
  Cell curr_cel_ = {};
  std::unordered_map<int32_t, std::vector<Cell>> cells_y_ = {};
  std::vector<Span> spans_ = {};
  std::array<glm::ivec2, 32 * 3 + 1> bez_stack_ = {};
  std::array<size_t, 32> lev_stack_ = {};
  bool curr_cell_invalid = true;
  bool even_odd = false;
  int32_t x_ = -1;
  int32_t y_ = -1;
  int32_t ex_ = -1;
  int32_t ey_ = -1;
  int32_t min_ex_ = std::numeric_limits<int32_t>::max();
  int32_t min_ey_ = std::numeric_limits<int32_t>::max();
  int32_t max_ex_ = std::numeric_limits<int32_t>::min();
  int32_t max_ey_ = std::numeric_limits<int32_t>::min();
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_SW_SW_RASTER_HPP