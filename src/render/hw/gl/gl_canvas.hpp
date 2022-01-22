#ifndef SKITY_SRC_RENDER_HW_GL_GL_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_GL_GL_CANVAS_HPP

#include <memory>

#include "src/render/hw/gl/gl_pipeline.hpp"
#include "src/render/hw/hw_canvas.hpp"

namespace skity {

class GLCanvas : public HWCanvas {
 public:
  GLCanvas(Matrix mvp, uint32_t width, uint32_t height, float density);
  ~GLCanvas() override = default;

 protected:
  void OnInit(GPUContext* ctx) override;
  std::unique_ptr<HWPipeline> CreatePipeline() override;
  std::unique_ptr<HWTexture> GenerateTexture() override;
  std::unique_ptr<HWFontTexture> GenerateFontTexture(
      Typeface* typeface) override;
  std::unique_ptr<HWRenderTarget> GenerateBackendRenderTarget(
      uint32_t width, uint32_t height) override;

 private:
  GPUContext* ctx_;
  GLPipeline* pipeline_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_GL_CANVAS_HPP