
#ifndef SKITY_SRC_RENDER_GL_GL_CANVAS2_HPP
#define SKITY_SRC_RENDER_GL_GL_CANVAS2_HPP

#include <memory>
#include <skity/geometry/point.hpp>
#include <skity/render/canvas.hpp>

namespace skity {

class GLMesh;
class GLVertex2;

class GLCanvas2 : public Canvas {
 public:
  GLCanvas2(Matrix const& mvp, int32_t width, int32_t height);
  ~GLCanvas2() override = default;

 protected:
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

 private:
  Matrix mvp_;
  int32_t width_;
  int32_t height_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_CANVAS2_HPP
