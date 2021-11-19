#ifndef SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP

#include <array>
#include <glm/glm.hpp>
#include <skity/graphic/paint.hpp>
#include <vector>

namespace skity {

class HWMesh;

class HWGeometryRaster {
 public:
  HWGeometryRaster(HWMesh* mesh, Paint const& paint, bool msaa);
  ~HWGeometryRaster() = default;

  std::array<uint32_t, 2> RasterLine(glm::vec2 const& p0, glm::vec2 const& p1);

 private:
  void HandleLineCap(glm::vec2 const& p0, glm::vec2 const& p1,
                     glm::vec2 const& out_dir, float stroke_radius);

  std::array<glm::vec2, 4> ExpandLine(glm::vec2 const& p0, glm::vec2 const& p1);

  uint32_t AppendLineVertex(glm::vec2 const& p, float v1);
  uint32_t AppendCircleVertex(glm::vec2 const& p, glm::vec2 const& center);

  void AppendRect(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

  std::array<uint32_t, 2> FlushIndexBuffer();

 private:
  HWMesh* mesh_;
  Paint paint_;
  bool msaa_;
  std::vector<uint32_t> index_buffer_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP