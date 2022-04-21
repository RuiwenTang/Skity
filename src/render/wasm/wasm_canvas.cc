#include <skity/gpu/gpu_context.hpp>
#include <skity/render/canvas.hpp>
#include <string>

#ifdef SKITY_WASM
#include <EGL/egl.h>
#include <emscripten/html5_webgl.h>

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeWebGLCanvas(std::string const& name,
                                                uint32_t width, uint32_t height,
                                                float density) {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  attrs.stencil = 1;
  attrs.enableExtensionsByDefault = 1;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context =
      emscripten_webgl_create_context(name.c_str(), &attrs);

  double pdr = emscripten_get_device_pixel_ratio();

  emscripten_set_element_css_size(name.c_str(), width, height);
  emscripten_set_canvas_element_size(name.c_str(), width * pdr, height * pdr);

  emscripten_webgl_make_context_current(context);

  GPUContext ctx(GPUBackendType::kWebGL2,
                 (void*)emscripten_webgl_get_proc_address);

  return MakeHardwareAccelationCanvas(width * pdr, height * pdr, density, &ctx);
}

}  // namespace skity

#endif