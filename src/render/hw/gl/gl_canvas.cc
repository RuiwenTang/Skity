#include "src/render/hw/gl/gl_canvas.hpp"

#include "src/render/hw/gl/gl_font_texture.hpp"
#include "src/render/hw/gl/gl_interface.hpp"
#include "src/render/hw/gl/gl_render_target.hpp"
#include "src/render/hw/gl/gl_texture.hpp"

#ifdef SKITY_ANDROID
#include <cstdio>
#endif

namespace skity {

GLCanvas::GLCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : HWCanvas(mvp, width, height, density) {}

void GLCanvas::OnInit(GPUContext* ctx) {
  ctx_ = ctx;
#if defined(__ANDROID__) || defined(ANDROID)
  const char* version;
  std::vector<const char*> prefixes = {
      "OpenGL ES-CM ",
      "OpenGL ES-CL ",
      "OpenGL ES ",
  };

  PFNGLGETSTRINGPROC gl_get_string = reinterpret_cast<PFNGLGETSTRINGPROC>(
          ((GLInterface::GLGetProc)ctx->proc_loader)("glGetString"));
  version = (const char*)gl_get_string(GL_VERSION);

  for (auto pref : prefixes) {
    size_t length = std::strlen(pref);
    if (std::strncmp(version, pref, length) == 0) {
      version += length;
      break;
    }
  }

  std::sscanf(version, "%d.%d", &gl_major_, &gl_minor_);
#endif
}

bool GLCanvas::SupportGeometryShader() {
#if defined(__ANDROID__) || defined(ANDROID)
  return gl_major_ == 3 && gl_minor_ == 2;
#else
  // FIXME
  // enable GS Shader by default in OpenGL
  return true;
#endif
}

std::unique_ptr<HWRenderer> GLCanvas::CreateRenderer() {
  auto renderer = std::make_unique<GLRenderer>(ctx_, SupportGeometryShader());
  renderer->Init();

  gl_renderer_ = renderer.get();

#ifdef SKITY_WASM
  GL_CALL(Viewport, 0, 0, onGetWidth(), onGetHeight());
#endif

  return renderer;
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

  // always enable multisample fbo
  fbo->SetEnableMultiSample(true);

  fbo->Init();

  return fbo;
}

}  // namespace skity