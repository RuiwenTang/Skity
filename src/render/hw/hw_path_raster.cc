#include "src/render/hw/hw_path_raster.hpp"

#include "src/render/hw/hw_mesh.hpp"

namespace skity {


void HWPathRaster::OnBeginPath() { ResetRaster(); }

void HWPathRaster::OnEndPath() {
  
}

void HWPathRaster::OnMoveTo(glm::vec2 const& p) {}

void HWPathRaster::OnLineTo(glm::vec2 const& p1, glm::vec2 const& p2) {}

void HWPathRaster::OnQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                            glm::vec2 const& p3) {}

}  // namespace skity