#include "src/render/hw/gl/gl_pipeline.hpp"

#include "src/render/hw/gl/gl_interface.hpp"

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

GLPipeline::GLPipeline(void* ctx) : HWPipeline(), ctx_(ctx) {}

GLPipeline::~GLPipeline() {}

void GLPipeline::Init() {
  GLInterface::InitGlobalInterface(ctx_);
  InitShader();
  InitBufferObject();
}

void GLPipeline::Bind() {
  shader_->Bind();
  BindBuffers();
}

void GLPipeline::UnBind() {
  UnBindBuffers();
  shader_->UnBind();
}

void GLPipeline::SetViewProjectionMatrix(glm::mat4 const& mvp) {
  shader_->SetMVP(mvp);
}

void GLPipeline::SetModelMatrix(glm::mat4 const& matrix) {
  shader_->SetTransformMatrix(matrix);
}

void GLPipeline::SetPipelineMode(HWPipelineMode mode) {
  int32_t color_type = static_cast<int32_t>(mode);
  shader_->SetColorType(color_type);
}

void GLPipeline::SetStrokeWidth(float width) { shader_->SetStrokeWidth(width); }

void GLPipeline::SetUniformColor(const glm::vec4& color) {
  shader_->SetUniformColor(color);
}

void GLPipeline::SetGradientBoundInfo(const glm::vec4& info) {}

void GLPipeline::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {}

void GLPipeline::SetColors(const std::vector<Color4f>& colors) {}

void GLPipeline::SetPositions(const std::vector<float>& pos) {}

void GLPipeline::UploadVertexBuffer(void* data, size_t data_size) {
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
  if (data_size > buffer_sizes_[0]) {
    // resize gpu buffer
    buffer_sizes_[0] = data_size;
    GL_CALL(BufferData, GL_ARRAY_BUFFER, data_size, nullptr, GL_STREAM_DRAW);
  }

  GL_CALL(BufferSubData, GL_ARRAY_BUFFER, 0, data_size, data);
}

void GLPipeline::UploadIndexBuffer(void* data, size_t data_size) {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
  if (data_size > buffer_sizes_[1]) {
    // resize gpu buffer
    buffer_sizes_[1] = data_size;
    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, data_size, nullptr,
            GL_STREAM_DRAW);
  }

  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, data_size, data);
}

void GLPipeline::EnableStencilTest() { GL_CALL(Enable, GL_STENCIL_TEST); }

void GLPipeline::DisableStencilTest() { GL_CALL(Disable, GL_STENCIL_TEST); }

void GLPipeline::DisableColorOutput() { GL_CALL(ColorMask, 0, 0, 0, 0); }

void GLPipeline::EnableColorOutput() { GL_CALL(ColorMask, 1, 1, 1, 1); }

void GLPipeline::UpdateStencilMask(uint8_t write_mask) {
  GL_CALL(StencilMask, write_mask);
}

void GLPipeline::UpdateStencilOp(HWStencilOp op) {
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, hw_stencil_op_to_gl(op));
}

void GLPipeline::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                   uint32_t compare_mask) {
  GL_CALL(StencilFunc, hw_stencil_func_to_gl(func), value, compare_mask);
}

void GLPipeline::DrawIndex(uint32_t start, uint32_t count) {
  GL_CALL(DrawElements, GL_TRIANGLES, count, GL_UNSIGNED_INT,
          (void*)(start * sizeof(GLuint)));
}

void GLPipeline::InitShader() { shader_ = GLShader::CreatePipelineShader(); }

void GLPipeline::InitBufferObject() {
  GL_CALL(GenVertexArrays, 1, &vao_);
  GL_CALL(GenBuffers, 2, buffers_.data());
}

void GLPipeline::BindBuffers() {
  GL_CALL(BindVertexArray, vao_);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);

  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void*)0);
  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void*)(2 * sizeof(float)));

  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
}

void GLPipeline::UnBindBuffers() { GL_CALL(BindVertexArray, 0); }

}  // namespace skity