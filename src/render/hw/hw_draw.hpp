#ifndef SKITY_SRC_RENDER_HW_HW_DRAW_HPP
#define SKITY_SRC_RENDER_HW_HW_DRAW_HPP

#include <cstdint>
#include <glm/glm.hpp>
#include <skity/effect/mask_filter.hpp>
#include <skity/geometry/rect.hpp>
#include <src/utils/lazy.hpp>
#include <vector>

namespace skity {

struct HWDrawRange {
  uint32_t start = 0;
  uint32_t count = 0;
};

class HWPipeline;
class HWTexture;
class HWRenderTarget;

class HWDraw {
 public:
  HWDraw(HWPipeline* pipeline, bool has_clip, bool clip_stencil = false)
      : pipeline_(pipeline), has_clip_(has_clip), clip_stencil_(clip_stencil) {}
  virtual ~HWDraw() = default;

  virtual void Draw();

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

  void SetClearStencilClip(bool clear);

  void SetTexture(HWTexture* texture);

  void SetFontTexture(HWTexture* font_texture);

  void SetGlobalAlpha(float alpha);

 protected:
  HWPipeline* GetPipeline() { return pipeline_; }
  bool HasClip() { return has_clip_; }

  const Lazy<glm::mat4>& TransformMatrix() const { return transform_matrix_; }

 private:
  void DoStencilIfNeed();
  void DoColorFill();
  void DoStencilBufferMove();

  void DoStencilBufferMoveInternal();

 private:
  HWPipeline* pipeline_;
  bool has_clip_;
  bool clip_stencil_;
  bool clear_stencil_clip_ = false;
  uint32_t pipeline_type_ = 0;
  uint32_t pipeline_mode_ = 0;
  HWDrawRange stencil_front_range_ = {};
  HWDrawRange stencil_back_range_ = {};
  HWDrawRange color_range_ = {};
  Lazy<float> stroke_width_ = {};
  Lazy<glm::vec4> uniform_color_ = {};
  Lazy<glm::mat4> transform_matrix_ = {};
  Lazy<glm::vec4> gradient_bounds_ = {};
  Lazy<float> global_alpha_ = {};
  std::vector<glm::vec4> gradient_colors_ = {};
  std::vector<float> gradient_stops_ = {};
  HWTexture* texture_ = {};
  HWTexture* font_texture_ = {};
};

class PostProcessDraw : public HWDraw {
 public:
  PostProcessDraw(HWRenderTarget* render_target,
                  std::vector<std::unique_ptr<HWDraw>> draw_list,
                  Rect const& bounds, HWPipeline* pipeline, bool has_clip,
                  bool clip_stencil = false);

  PostProcessDraw(HWRenderTarget* render_target, std::unique_ptr<HWDraw> op,
                  Rect const& bounds, HWPipeline* pipeline, bool has_clip,
                  bool clip_stencil = false);

  ~PostProcessDraw() override;

  void SetBlurStyle(BlurStyle style) { blur_style_ = style; }

  void SetBlurRadius(float sigma) { blur_radius_ = sigma; }

  void Draw() override;

 private:
  void DrawToRenderTarget();

  void DoFilter();

  void DrawToCanvas();

  void SaveTransform();
  void RestoreTransform();

 private:
  HWRenderTarget* render_target_ = {};
  std::vector<std::unique_ptr<HWDraw>> draw_list_ = {};
  Rect bounds_ = {};
  BlurStyle blur_style_ = BlurStyle::kNormal;
  float blur_radius_ = 0.f;
  glm::mat4 saved_mvp_ = {};
  glm::mat4 saved_transform_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_DRAW_HPP