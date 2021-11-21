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
  HWGeometryRaster(HWMesh* mesh, Paint const& paint);
  virtual ~HWGeometryRaster() = default;

  void RasterLine(glm::vec2 const& p0, glm::vec2 const& p1);

  void ResetRaster();
  void FlushRaster();

  uint32_t StencilFrontStart() const { return stencil_front_start_; }
  uint32_t StencilFrontCount() const { return stencil_front_count_; }
  uint32_t StencilBackStart() const { return stencil_back_start_; }
  uint32_t StencilBackCount() const { return stencil_back_count_; }
  uint32_t ColorStart() const { return color_start_; }
  uint32_t ColorCount() const { return color_count_; }

 protected:
  enum BufferType {
    kStencilFront,
    kStencilBack,
    kColor,
  };

  void SetBufferType(BufferType type) { buffer_type_ = type; }

  float StrokeWidth() const { return paint_.getStrokeWidth(); }
  float StrokeMiter() const { return paint_.getStrokeMiter(); }
  Paint::Cap LineCap() const { return paint_.getStrokeCap(); }
  Paint::Join LineJoin() const { return paint_.getStrokeJoin(); }

 protected:
  void HandleLineCap(glm::vec2 const& center, glm::vec2 const& p0,
                     glm::vec2 const& p1, glm::vec2 const& out_dir,
                     float stroke_radius);

  std::array<glm::vec2, 4> ExpandLine(glm::vec2 const& p0, glm::vec2 const& p1,
                                      float stroke_radius);

  uint32_t AppendLineVertex(glm::vec2 const& p);
  uint32_t AppendCircleVertex(glm::vec2 const& p, glm::vec2 const& center);

  void AppendRect(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

  void AppendFrontTriangle(uint32_t a, uint32_t b, uint32_t c);
  void AppendBackTriangle(uint32_t a, uint32_t b, uint32_t c);

 private:
  std::vector<uint32_t>& CurrentIndexBuffer();
 private:
  HWMesh* mesh_;
  Paint paint_;
  BufferType buffer_type_ = kColor;

  uint32_t stencil_front_start_ = {};
  uint32_t stencil_front_count_ = {};
  uint32_t stencil_back_start_ = {};
  uint32_t stencil_back_count_ = {};
  uint32_t color_start_ = {};
  uint32_t color_count_ = {};

  std::vector<uint32_t> stencil_front_buffer_ = {};
  std::vector<uint32_t> stencil_back_buffer_ = {};
  std::vector<uint32_t> color_buffer_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP