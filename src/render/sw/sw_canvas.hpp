#ifndef SKITY_SRC_RENDER_SW_SW_CANVAS_HPP
#define SKITY_SRC_RENDER_SW_SW_CANVAS_HPP

#include <skity/config.hpp>
#include <skity/render/canvas.hpp>

#ifndef SKITY_CPU
#error "NOT Enable CPU Backend"
#endif

namespace skity {

class Bitmap;

class SWCanvas : public Canvas {
 public:
  SWCanvas(Bitmap* bitmap);
  ~SWCanvas() override = default;

 protected:
  void onDrawLine(float x0, float y0, float x1, float y1,
                  Paint const& paint) override;

  void onDrawRect(Rect const& rect, Paint const& paint) override;

  void onClipPath(const Path& path, ClipOp op) override;

  void onDrawPath(const Path& path, const Paint& paint) override;

  void onDrawBlob(const TextBlob* blob, float x, float y,
                  Paint const& paint) override;

  void onSave() override;

  void onRestore() override;

  void onRestoreToCount(int saveCount) override;

  void onTranslate(float dx, float dy) override;

  void onScale(float sx, float sy) override;

  void onRotate(float degree) override;

  void onRotate(float degree, float px, float py) override;

  void onConcat(const Matrix& matrix) override;

  void onFlush() override;

  uint32_t onGetWidth() const override;

  uint32_t onGetHeight() const override;

  void onUpdateViewport(uint32_t width, uint32_t height) override;

 private:
  Bitmap* bitmap_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_SW_SW_CANVAS_HPP