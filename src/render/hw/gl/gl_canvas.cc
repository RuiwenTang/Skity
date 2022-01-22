#include "src/render/hw/gl/gl_canvas.hpp"

#include "src/render/hw/gl/gl_font_texture.hpp"
#include "src/render/hw/gl/gl_render_target.hpp"
#include "src/render/hw/gl/gl_texture.hpp"

namespace skity {

GLCanvas::GLCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : HWCanvas(mvp, width, height, density) {}

void GLCanvas::OnInit(GPUContext* ctx) { ctx_ = ctx; }

std::unique_ptr<HWPipeline> GLCanvas::CreatePipeline() {
  auto pipeline = std::make_unique<GLPipeline>(ctx_);
  pipeline->Init();

  pipeline_ = pipeline.get();

  return pipeline;
}

std::unique_ptr<HWTexture> GLCanvas::GenerateTexture() {
  return std::make_unique<GLTexture>();
}

std::unique_ptr<HWFontTexture> GLCanvas::GenerateFontTexture(
    Typeface* typeface) {
  return std::make_unique<GLFontTexture>(typeface);
}

std::unique_ptr<HWRenderTarget> GLCanvas::GenerateBackendRenderTarget(
    uint32_t width, uint32_t height) {
  auto fbo = std::make_unique<GLRenderTarget>(width, height);

  fbo->Init();

  return fbo;
}

}  // namespace skity