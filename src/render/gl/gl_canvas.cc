#include "src/render/gl/gl_canvas.hpp"

namespace skity {

GLCanvas::GLCanvas() { Init(); }

void GLCanvas::Init() {
  InitShader();
  InitMesh();
}

void GLCanvas::InitShader() {
  stencil_shader_ = GLShader::CreateStencilShader();
}

void GLCanvas::InitMesh() {
  vertex_ = std::make_unique<GLVertex>();
  mesh_ = std::make_unique<GLMesh>();
  mesh_->Init();
}

}  // namespace skity