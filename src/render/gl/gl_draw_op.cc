#include "src/render/gl/gl_draw_op.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLDrawStencilOp::GLDrawStencilOp(uint32_t front_start, uint32_t front_count,
                                 uint32_t back_start, uint32_t back_count,
                                 StencilShader* stencil_shader)
    : GLDrawOp(),
      front_start_(front_start),
      front_count_(front_count),
      back_start_(back_start),
      back_count_(back_count),
      stencil_shader_(stencil_shader) {}

void GLDrawStencilOp::Draw() {
  DoStencil();
  DoDraw();
}

void GLDrawStencilOp::DoStencil() {
    GetStencilShader()->Bind();
    
    OnBeforeDoStencil();
    DrawFront();
    DrawBack();
    
    GetStencilShader()->UnBind();
}

void GLDrawStencilOp::DoDraw() {
    OnBeforeDoDraw();
    DrawFront();
    DrawBack();
}

}  // namespace skity
