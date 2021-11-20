#ifndef SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP

#include <vector>

#include "src/render/hw/hw_path_visitor.hpp"

namespace skity {

class HWMesh;

class HWPathRaster : public HWPathVisitor {
 public:
  HWPathRaster(HWMesh* mesh, Paint const& paint) : HWPathVisitor(mesh, paint) {}
  ~HWPathRaster() override = default;

 protected:
  void OnBeginPath() override;
  void OnEndPath() override;
  void OnMoveTo(glm::vec2 const& p) override;

  void OnLineTo(glm::vec2 const& p1, glm::vec2 const& p2) override;

  void OnQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                glm::vec2 const& p3) override;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP