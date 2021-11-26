#include "src/render/hw/gl/gl_canvas.hpp"

#include "src/render/hw/gl/gl_texture.hpp"

namespace skity {

GLCanvas::GLCanvas(Matrix mvp, uint32_t width, uint32_t height)
    : HWCanvas(mvp, width, height) {}

void GLCanvas::OnInit(void* ctx) {
  pipeline_ = std::make_unique<GLPipeline>(ctx);
  pipeline_->Init();
}

HWPipeline* GLCanvas::GetPipeline() { return pipeline_.get(); }

std::unique_ptr<HWTexture> GLCanvas::GenerateTexture() {
  return std::make_unique<GLTexture>();
}

}  // namespace skity