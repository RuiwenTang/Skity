#ifndef SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP

#include <vector>

#include "src/render/hw/hw_path_visitor.hpp"

namespace skity {

class HWMesh;

class HWPathRaster : public HWPathVisitor {
 public:
  HWPathRaster(Paint const& paint, HWMesh* mesh)
      : HWPathVisitor(paint), mesh_(mesh) {}
  ~HWPathRaster() override = default;

 protected:
  void AddStencilFront(uint32_t a, uint32_t b, uint32_t c);
  void AddStencilBack(uint32_t a, uint32_t b, uint32_t c);
  void AddColor(uint32_t a, uint32_t b, uint32_t c);
  void AddAA(uint32_t a, uint32_t b, uint32_t c);

  uint32_t AppendVertex(float x, float y, float mix, float u, float v);

  void OnBeginPath() override;
  void OnEndPath() override;

 private:
  HWMesh* mesh_;
  uint32_t stencil_front_start_ = {};
  uint32_t stencil_front_count_ = {};
  uint32_t stencil_back_start_ = {};
  uint32_t stencil_back_count_ = {};
  uint32_t color_start_ = {};
  uint32_t color_count_ = {};
  uint32_t aa_start_ = {};
  uint32_t aa_count_ = {};

  std::vector<uint32_t> stencil_front_buffer_ = {};
  std::vector<uint32_t> stencil_back_buffer_ = {};
  std::vector<uint32_t> color_buffer_ = {};
  std::vector<uint32_t> aa_buffer_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP