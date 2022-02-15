#ifndef SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP

#include <array>
#include <glm/glm.hpp>
#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <vector>

#include "src/utils/lazy.hpp"

namespace skity {

class HWMesh;

class HWGeometryRaster {
 public:
  HWGeometryRaster(HWMesh* mesh, Paint const& paint, bool use_gs);
  virtual ~HWGeometryRaster() = default;

  void RasterLine(glm::vec2 const& p0, glm::vec2 const& p1);
  void RasterRect(Rect const& rect);
  void FillCircle(float cx, float cy, float radius);
  void FillTextRect(glm::vec4 const& bounds, glm::vec2 const& uv_lt,
                    glm::vec2 const& uv_rb);

  void ResetRaster();
  void FlushRaster();

  uint32_t StencilFrontStart() const { return stencil_front_start_; }
  uint32_t StencilFrontCount() const { return stencil_front_count_; }
  uint32_t StencilBackStart() const { return stencil_back_start_; }
  uint32_t StencilBackCount() const { return stencil_back_count_; }
  uint32_t ColorStart() const { return color_start_; }
  uint32_t ColorCount() const { return color_count_; }

  Rect RasterBounds() const;

  bool UseGeometryShader() const { return use_gs_; }

 protected:
  enum BufferType {
    kStencilFront,
    kStencilBack,
    kColor,
  };

  void SetBufferType(BufferType type) { buffer_type_ = type; }

  float StrokeWidth() const;
  float StrokeMiter() const { return paint_.getStrokeMiter(); }
  Paint::Cap LineCap() const { return paint_.getStrokeCap(); }
  Paint::Join LineJoin() const { return paint_.getStrokeJoin(); }

  void HandleLineCap(glm::vec2 const& center, glm::vec2 const& p0,
                     glm::vec2 const& p1, glm::vec2 const& out_dir,
                     float stroke_radius);

  std::array<glm::vec2, 4> ExpandLine(glm::vec2 const& p0, glm::vec2 const& p1,
                                      float stroke_radius);

  uint32_t AppendLineVertex(glm::vec2 const& p);
  uint32_t AppendCircleVertex(glm::vec2 const& p, glm::vec2 const& center);
  uint32_t AppendVertex(float x, float y, float mix, float u, float v);

  void AppendRect(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

  void AppendFrontTriangle(uint32_t a, uint32_t b, uint32_t c);
  void AppendBackTriangle(uint32_t a, uint32_t b, uint32_t c);

  void FillRect(Rect const& rect);
  void StrokeRect(Rect const& rect);

  // used for convexity polygon
  void SwitchStencilToColor();

 private:
  std::vector<uint32_t>& CurrentIndexBuffer();
  void ExpandBounds(glm::vec2 const& p);

 private:
  HWMesh* mesh_;
  Paint paint_;
  bool use_gs_;
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

  Lazy<glm::vec4> bounds_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP