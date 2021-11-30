#ifndef SKITY_SRC_RENDER_HW_HW_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_HW_CANVAS_HPP

#include <map>
#include <memory>
#include <skity/geometry/point.hpp>
#include <skity/render/canvas.hpp>
#include <vector>

#include "src/render/hw/hw_canvas_state.hpp"
#include "src/render/hw/hw_draw.hpp"
#include "src/render/hw/hw_font_texture.hpp"
#include "src/render/hw/hw_texture.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class Pixmap;
class Typeface;

class HWMesh;
class HWPipeline;

/**
 * @class HWCanvas
 *  Base class for all hardware canvas implementation, use MSAA for anti-alias
 */
class HWCanvas : public Canvas {
 public:
  HWCanvas(Matrix mvp, uint32_t width, uint32_t height);
  ~HWCanvas() override;

  void Init(void* ctx);

 protected:
  virtual void OnInit(void* ctx) = 0;

  virtual HWPipeline* GetPipeline() = 0;
  virtual std::unique_ptr<HWTexture> GenerateTexture() = 0;
  virtual std::unique_ptr<HWFontTexture> GenerateFontTexture(
      Typeface* typeface) = 0;

  void onDrawLine(float x0, float y0, float x1, float y1,
                  Paint const& paint) override;

  void onDrawCircle(float cx, float cy, float radius,
                    Paint const& paint) override;

  void onDrawRect(Rect const& rect, Paint const& paint) override;

  void onClipPath(const Path& path, ClipOp op) override;

  void onDrawPath(const Path& path, const Paint& paint) override;

  void onDrawGlyphs(const std::vector<GlyphInfo>& glyphs, Typeface* typeface,
                    const Paint& paint) override;

  void onSave() override;

  void onRestore() override;

  void onTranslate(float dx, float dy) override;

  void onScale(float sx, float sy) override;

  void onRotate(float degree) override;

  void onRotate(float degree, float px, float py) override;

  void onConcat(const Matrix& matrix) override;

  void onFlush() override;

  uint32_t onGetWidth() const override;

  uint32_t onGetHeight() const override;

  void onUpdateViewport(uint32_t width, uint32_t height) override;

  HWMesh* GetMesh();

 private:
  std::unique_ptr<HWDraw> GenerateOp();

  std::unique_ptr<HWDraw> GenerateColorOp(Paint const& paint,
                                          bool stroke = false,
                                          Rect const& = {});

  HWTexture* QueryTexture(Pixmap* pixmap);
  HWFontTexture* QueryFontTexture(Typeface* typeface);

 private:
  Matrix mvp_;
  uint32_t width_;
  uint32_t height_;
  HWCanvasState state_;
  std::unique_ptr<HWMesh> mesh_;
  Lazy<float> global_alpha_ = {};
  std::vector<std::unique_ptr<HWDraw>> draw_ops_ = {};
  std::map<Pixmap*, std::unique_ptr<HWTexture>> image_texture_store_ = {};
  std::map<Typeface*, std::unique_ptr<HWFontTexture>> font_texture_store_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_CANVAS_HPP