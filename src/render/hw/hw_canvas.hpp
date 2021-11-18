#ifndef SKITY_SRC_RENDER_HW_HW_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_HW_CANVAS_HPP

#include <memory>
#include <skity/geometry/point.hpp>
#include <skity/render/canvas.hpp>
#include <vector>

#include "src/render/hw/hw_canvas_state.hpp"

namespace skity {

class HWMesh;
class HWShader;

/**
 * @class HWCanvas
 *  Base class for all hardware canvas implementation
 */
class HWCanvas : public Canvas {
 public:
  HWCanvas(Matrix mvp, uint32_t width, uint32_t height);
  ~HWCanvas() override;

  void Init();

 protected:
  virtual void OnInit() = 0;

  virtual HWShader* GetShader() = 0;

  void onClipRect(Rect const& rect, ClipOp op) override;

  void onDrawLine(float x0, float y0, float x1, float y1,
                  Paint const& paint) override;

  void onDrawCircle(float cx, float cy, float radius,
                    Paint const& paint) override;

  void onDrawOval(Rect const& oval, Paint const& paint) override;

  void onDrawRect(Rect const& rect, Paint const& paint) override;

  void onDrawRRect(RRect const& rrect, Paint const& paint) override;

  void onDrawRoundRect(Rect const& rect, float rx, float ry,
                       Paint const& paint) override;

  void onClipPath(const Path& path, ClipOp op) override;

  void onDrawPath(const Path& path, const Paint& paint) override;

  void onDrawGlyphs(const std::vector<GlyphInfo>& glyphs,
                    const Typeface* typeface, const Paint& paint) override;

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
  Matrix mvp_;
  uint32_t width_;
  uint32_t height_;
  HWCanvasState state_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_CANVAS_HPP