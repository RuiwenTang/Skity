#ifndef SKITY_SRC_RENDER_HW_HW_DRAW_HPP
#define SKITY_SRC_RENDER_HW_HW_DRAW_HPP

#include <cstdint>
#include <glm/glm.hpp>
#include <src/utils/lazy.hpp>
#include <vector>

namespace skity {

struct HWDrawRange {
  uint32_t start = 0;
  uint32_t count = 0;
};

class HWPipeline;

class HWDraw {
 public:
  HWDraw(HWPipeline* pipeline, bool has_clip, bool clip_stencil = false)
      : pipeline_(pipeline), has_clip_(has_clip), clip_stencil_(clip_stencil) {}
  virtual ~HWDraw() = default;

  void Draw();

  void SetPipelineColorMode(uint32_t mode);

  void SetStencilRange(HWDrawRange const& front_range,
                       HWDrawRange const& back_range);

  void SetColorRange(HWDrawRange const& color_range);

  void SetStrokeWidth(float width);

  void SetUniformColor(glm::vec4 const& color);

  void SetTransformMatrix(glm::mat4 const& matrix);

  void SetGradientBounds(glm::vec2 const& p0, glm::vec2 const& p1);

  void SetGradientColors(std::vector<glm::vec4> const& colors);

  void SetGradientPositions(std::vector<float> const& pos);

 protected:
  HWPipeline* GetPipeline() { return pipeline_; }
  bool HasClip() { return has_clip_; }

 private:
  void DoStencilIfNeed();
  void DoColorFill();
  void DoStencilBufferMove();

 private:
  HWPipeline* pipeline_;
  bool has_clip_;
  bool clip_stencil_;
  uint32_t pipeline_mode_ = 0;
  HWDrawRange stencil_front_range_ = {};
  HWDrawRange stencil_back_range_ = {};
  HWDrawRange color_range_ = {};
  Lazy<float> stroke_width_ = {};
  Lazy<glm::vec4> uniform_color_ = {};
  Lazy<glm::mat4> transform_matrix_ = {};
  Lazy<glm::vec4> gradient_bounds_ = {};
  std::vector<glm::vec4> gradient_colors_ = {};
  std::vector<float> gradient_stops_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_DRAW_HPP