#include "src/render/hw/gl/gl_pipeline.hpp"

#include "src/render/hw/gl/gl_interface.hpp"

namespace skity {

GLPipeline::GLPipeline(void* ctx) : HWPipeline(), ctx_(ctx) {}

GLPipeline::~GLPipeline() {}

void GLPipeline::Init() {
  GLInterface::InitGlobalInterface(ctx_);
  InitShader();
  InitBufferObject();
}

void GLPipeline::Bind() {}

void GLPipeline::UnBind() {}

void GLPipeline::SetViewProjectionMatrix(glm::mat4 const& mvp) {}

void GLPipeline::SetModelMatrix(glm::mat4 const& matrix) {}

void GLPipeline::SetPipelineMode(HWPipelineMode mode) {}

void GLPipeline::SetStrokeWidth(float width) {}

void GLPipeline::SetUniformColor(const glm::vec4& color) {}

void GLPipeline::SetGradientBoundInfo(const glm::vec4& info) {}

void GLPipeline::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {}

void GLPipeline::SetColors(const std::vector<Color4f>& colors) {}

void GLPipeline::SetPositions(const std::vector<float>& pos) {}

void GLPipeline::UploadVertexBuffer(void* data, size_t data_size) {}

void GLPipeline::UploadIndexBuffer(void* data, size_t data_size) {}

void GLPipeline::EnableStencilTest() {}

void GLPipeline::DisableStencilTest() {}

void GLPipeline::DisableColorOutput() {}

void GLPipeline::EnableColorOutput() {}

void GLPipeline::UpdateStencilMask(uint8_t write_mask, uint8_t compare_mask) {}

void GLPipeline::UpdateStencilOp(HWStencilOp op) {}

void GLPipeline::UpdateStencilFunc(HWStencilFunc func, uint32_t value) {}

void GLPipeline::DrawIndex(uint32_t start, uint32_t count) {}

void GLPipeline::InitShader() { shader_ = GLShader::CreatePipelineShader(); }

void GLPipeline::InitBufferObject() {
  GL_CALL(GenVertexArrays, 1, &vao_);
  GL_CALL(GenBuffers, 2, buffers_.data());

  GL_CALL(BindVertexArray, vao_);
}

}  // namespace skity