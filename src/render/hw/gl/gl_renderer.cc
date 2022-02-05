#include "src/render/hw/gl/gl_renderer.hpp"

#include "src/render/hw/gl/gl_interface.hpp"
#include "src/render/hw/gl/gl_render_target.hpp"

namespace skity {

static GLenum hw_stencil_func_to_gl(HWStencilFunc func) {
  switch (func) {
    case HWStencilFunc::EQUAL:
      return GL_EQUAL;
    case HWStencilFunc::NOT_EQUAL:
      return GL_NOTEQUAL;
    case HWStencilFunc::ALWAYS:
      return GL_ALWAYS;
    case HWStencilFunc::LESS:
      return GL_LESS;
    case HWStencilFunc::LESS_OR_EQUAL:
      return GL_LEQUAL;
    case HWStencilFunc::GREAT_OR_EQUAL:
      return GL_GEQUAL;
  }

  return 0;
}

static GLenum hw_stencil_op_to_gl(HWStencilOp op) {
  switch (op) {
    case HWStencilOp::DECR_WRAP:
      return GL_DECR_WRAP;
    case HWStencilOp::INCR_WRAP:
      return GL_INCR_WRAP;
    case HWStencilOp::KEEP:
      return GL_KEEP;
    case HWStencilOp::REPLACE:
      return GL_REPLACE;
  }

  return 0;
}

static GLenum slot_to_gl_texture_unit(uint32_t slot) {
  switch (slot) {
    case 0:
      return GL_TEXTURE0;
    case 1:
      return GL_TEXTURE1;
  }

  return GL_TEXTURE0;
}

GLRenderer::GLRenderer(GPUContext* ctx) : HWRenderer(), ctx_(ctx) {}

GLRenderer::~GLRenderer() {
  GL_CALL(DeleteBuffers, buffers_.size(), buffers_.data());
  GL_CALL(DeleteVertexArrays, 1, &vao_);
}

void GLRenderer::Init() {
  GLInterface::InitGlobalInterface(ctx_->proc_loader);
  InitShader();
  InitBufferObject();
}

void GLRenderer::Destroy() {}

void GLRenderer::Bind() {
  shader_->Bind();
  shader_->SetUserTexture(0);
  shader_->SetFontTexture(1);
  BindBuffers();
}

void GLRenderer::UnBind() {
  UnBindBuffers();
  shader_->UnBind();
}

void GLRenderer::SetViewProjectionMatrix(glm::mat4 const& mvp) {
  HWRenderer::SetViewProjectionMatrix(mvp);
  shader_->SetMVP(mvp);
}

void GLRenderer::SetModelMatrix(glm::mat4 const& matrix) {
  HWRenderer::SetModelMatrix(matrix);
  shader_->SetTransformMatrix(matrix);
}

void GLRenderer::SetPipelineColorMode(HWPipelineColorMode mode) {
  int32_t color_type = static_cast<int32_t>(mode);
  shader_->SetColorType(color_type);
}

void GLRenderer::SetStrokeWidth(float width) { shader_->SetStrokeWidth(width); }

void GLRenderer::SetUniformColor(const glm::vec4& color) {
  shader_->SetUniformColor(color);
}

void GLRenderer::SetGradientBoundInfo(const glm::vec4& info) {
  shader_->SetGradientBoundInfo(info);
}

void GLRenderer::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {
  shader_->SetGradientCountInfo(color_count, pos_count);
}

void GLRenderer::SetGradientColors(const std::vector<Color4f>& colors) {
  shader_->SetGradientColors(colors);
}

void GLRenderer::SetGradientPositions(const std::vector<float>& pos) {
  shader_->SetGradientPostions(pos);
}

void GLRenderer::UploadVertexBuffer(void* data, size_t data_size) {
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
  if (data_size > buffer_sizes_[0]) {
    // resize gpu buffer
    buffer_sizes_[0] = data_size;
    GL_CALL(BufferData, GL_ARRAY_BUFFER, data_size, nullptr, GL_STREAM_DRAW);
  }

  GL_CALL(BufferSubData, GL_ARRAY_BUFFER, 0, data_size, data);
}

void GLRenderer::UploadIndexBuffer(void* data, size_t data_size) {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
  if (data_size > buffer_sizes_[1]) {
    // resize gpu buffer
    buffer_sizes_[1] = data_size;
    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, data_size, nullptr,
            GL_STREAM_DRAW);
  }

  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, data_size, data);
}

void GLRenderer::SetGlobalAlpha(float alpha) { shader_->SetGlobalAlpha(alpha); }

void GLRenderer::EnableStencilTest() { GL_CALL(Enable, GL_STENCIL_TEST); }

void GLRenderer::DisableStencilTest() { GL_CALL(Disable, GL_STENCIL_TEST); }

void GLRenderer::DisableColorOutput() { GL_CALL(ColorMask, 0, 0, 0, 0); }

void GLRenderer::EnableColorOutput() { GL_CALL(ColorMask, 1, 1, 1, 1); }

void GLRenderer::UpdateStencilMask(uint8_t write_mask) {
  GL_CALL(StencilMask, write_mask);
}

void GLRenderer::UpdateStencilOp(HWStencilOp op) {
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, hw_stencil_op_to_gl(op));
}

void GLRenderer::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                   uint32_t compare_mask) {
  GL_CALL(StencilFunc, hw_stencil_func_to_gl(func), value, compare_mask);
}

void GLRenderer::DrawIndex(uint32_t start, uint32_t count) {
  GL_CALL(DrawElements, GL_TRIANGLES, count, GL_UNSIGNED_INT,
          (void*)(start * sizeof(GLuint)));
}

void GLRenderer::InitShader() { shader_ = GLShader::CreatePipelineShader(); }

void GLRenderer::InitBufferObject() {
  GL_CALL(GenVertexArrays, 1, &vao_);
  GL_CALL(GenBuffers, 2, buffers_.data());

  GL_CALL(BindVertexArray, vao_);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);

  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void*)0);
  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void*)(2 * sizeof(float)));

  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);

  GL_CALL(BindVertexArray, 0);
}

void GLRenderer::BindBuffers() { GL_CALL(BindVertexArray, vao_); }

void GLRenderer::UnBindBuffers() { GL_CALL(BindVertexArray, 0); }

void GLRenderer::BindTexture(HWTexture* /* unused */, uint32_t slot) {
  GL_CALL(ActiveTexture, slot_to_gl_texture_unit(slot));
}

void GLRenderer::BindRenderTarget(HWRenderTarget* render_target) {
  GLRenderTarget* fbo = (GLRenderTarget*)render_target;

  fbo->Bind();

  // save current viewport
  GL_CALL(GetIntegerv, GL_VIEWPORT, &saved_viewport_[0]);

  GL_CALL(Viewport, 0, 0, fbo->Width(), fbo->Height());
}

void GLRenderer::UnBindRenderTarget(HWRenderTarget* render_target) {
  GLRenderTarget* fbo = (GLRenderTarget*)render_target;

  fbo->UnBind();

  // restore saved viewport
  GL_CALL(Viewport, saved_viewport_[0], saved_viewport_[1], saved_viewport_[2],
          saved_viewport_[3]);
}

}  // namespace skity