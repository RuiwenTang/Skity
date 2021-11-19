#ifndef SKITY_SRC_RENDER_HW_HW_DRAW_HPP
#define SKITY_SRC_RENDER_HW_HW_DRAW_HPP

#include <cstdint>

namespace skity {

enum class HWDrawType {
  STENCIL_FRONT,
  STENCIL_BACK,
  COLOR,
  AA,
};

struct HWDrawRange {
  uint32_t start = 0;
  uint32_t count = 0;
};

class HWPipeline;

class HWDraw {
 public:
  HWDraw(HWPipeline* pipeline, bool has_clip)
      : pipeline_(pipeline), has_clip_(has_clip) {}
  virtual ~HWDraw() = default;

  void Draw();

 protected:
  HWPipeline* GetPipeline() { return pipeline_; }
  bool HasClip() { return has_clip_; }

  virtual void OnBeforeDraw(bool has_clip) = 0;
  virtual void OnDraw(bool has_clip) = 0;
  virtual void OnEndDraw(bool has_clip) = 0;

 private:
  HWPipeline* pipeline_;
  bool has_clip_;
};

class FillColorDraw : public HWDraw {
 public:
  FillColorDraw(HWPipeline* pipeline, bool has_clip, HWDrawRange const& range,
                bool stencil_discard = false)
      : HWDraw(pipeline, has_clip),
        draw_range_(range),
        stencil_discard_(stencil_discard) {}

  ~FillColorDraw() override = default;

 protected:
  void OnBeforeDraw(bool has_clip) override;
  void OnDraw(bool has_clip) override;

 private:
  HWDrawRange draw_range_;
  bool stencil_discard_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_DRAW_HPP