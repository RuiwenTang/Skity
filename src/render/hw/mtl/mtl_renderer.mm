
#include <simd/simd.h>
#include "mtl_renderer.hpp"

#include "src/render/hw/mtl/mtl_render_target.hpp"
#include "skity/gpu/gpu_mtl_context.hpp"

namespace skity {

//static GLenum hw_stencil_func_to_gl(HWStencilFunc func) {
//  switch (func) {
//    case HWStencilFunc::EQUAL:
//      return GL_EQUAL;
//    case HWStencilFunc::NOT_EQUAL:
//      return GL_NOTEQUAL;
//    case HWStencilFunc::ALWAYS:
//      return GL_ALWAYS;
//    case HWStencilFunc::LESS:
//      return GL_LESS;
//    case HWStencilFunc::GREAT:
//      return GL_GREATER;
//    case HWStencilFunc::LESS_OR_EQUAL:
//      return GL_LEQUAL;
//    case HWStencilFunc::GREAT_OR_EQUAL:
//      return GL_GEQUAL;
//  }
//
//  return 0;
//}
//
//static GLenum hw_stencil_op_to_gl(HWStencilOp op) {
//  switch (op) {
//    case HWStencilOp::DECR_WRAP:
//      return GL_DECR_WRAP;
//    case HWStencilOp::INCR_WRAP:
//      return GL_INCR_WRAP;
//    case HWStencilOp::KEEP:
//      return GL_KEEP;
//    case HWStencilOp::REPLACE:
//      return GL_REPLACE;
//  }
//
//  return 0;
//}
//
//static GLenum slot_to_gl_texture_unit(uint32_t slot) {
//  switch (slot) {
//    case 0:
//      return GL_TEXTURE0;
//    case 1:
//      return GL_TEXTURE1;
//  }
//
//  return GL_TEXTURE0;
//}

MTLRenderer::MTLRenderer(std::shared_ptr<GPUContext> &ctx, bool use_gs)
: HWRenderer(), ctx_(ctx) {
    
    passthroughRenderPipeline_ = std::move(std::unique_ptr<MTLPassthroughRenderPipeline>(new MTLPassthroughRenderPipeline(ctx)));
    
}

MTLRenderer::~MTLRenderer() {
    //  GL_CALL(DeleteBuffers, buffers_.size(), buffers_.data());
    //  GL_CALL(DeleteVertexArrays, 1, &vao_);
}

void MTLRenderer::Init() {
    
    
    
    
    //  InitShader();
    //  InitBufferObject();
    //
    //  GL_CALL(GetIntegerv, GL_FRAMEBUFFER_BINDING, &root_fbo_);
}

void MTLRenderer::Destroy() {
    
}

void MTLRenderer::Bind() {
    //  shader_->Bind();
    //  shader_->SetUserTexture(0);
    //  shader_->SetFontTexture(1);
    //  BindBuffers();
}

void MTLRenderer::UnBind() {
    //  UnBindBuffers();
    //  shader_->UnBind();
}

void MTLRenderer::SetViewProjectionMatrix(glm::mat4 const& mvp) {
      HWRenderer::SetViewProjectionMatrix(mvp);
    passthroughRenderPipeline_->mvp_ = mvp;
}

void MTLRenderer::SetModelMatrix(glm::mat4 const& matrix) {
      HWRenderer::SetModelMatrix(matrix);
    //  shader_->SetTransformMatrix(matrix);
    passthroughRenderPipeline_->modelMat_ = matrix;
}

void MTLRenderer::SetPipelineColorMode(HWPipelineColorMode mode) {
      int32_t color_type = static_cast<int32_t>(mode);
    //  shader_->SetColorType(color_type);
    passthroughRenderPipeline_->color_type = color_type;
}

void MTLRenderer::SetStrokeWidth(float width) {
    //    shader_->SetStrokeWidth(width);
    passthroughRenderPipeline_->strokeWidth_ = width;
}

void MTLRenderer::SetUniformColor(const glm::vec4& color) {
    //  shader_->SetUniformColor(color);
    passthroughRenderPipeline_->uniform_color_ = color;
}

void MTLRenderer::SetGradientBoundInfo(const glm::vec4& info) {
    passthroughRenderPipeline_->gradientBoundInfo_ = info;
//      shader_->SetGradientBoundInfo(info);
//    assert(false);
}

void MTLRenderer::SetGradientCountInfo(int32_t color_count, int32_t pos_count) {
    //  shader_->SetGradientCountInfo(color_count, pos_count);
//    assert(false);
}

void MTLRenderer::SetGradientColors(const std::vector<Color4f>& colors) {
    //  shader_->SetGradientColors(colors);
//    assert(false);
    passthroughRenderPipeline_->gradientColors_ = colors;
}

void MTLRenderer::SetGradientPositions(const std::vector<float>& pos) {
    //  shader_->SetGradientPostions(pos);
//    assert(false);
    passthroughRenderPipeline_->gradientPos_ = pos;
}


void MTLRenderer::UploadVertexBuffer(void* data, size_t data_size) {
    
    GPUMTLContext *mtlctx = (GPUMTLContext*)ctx_.get();

    if ([passthroughRenderPipeline_->vertexBuffer_ length] >= data_size) {
        memcpy([passthroughRenderPipeline_->vertexBuffer_ contents], data, data_size);
    }
    else
    {
        id<MTLBuffer> buf = [mtlctx->mDevice newBufferWithBytes:data length:data_size options:MTLStorageModeShared];
        passthroughRenderPipeline_->vertexBuffer_ = buf;
    }
    //  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
    //  if (data_size > buffer_sizes_[0]) {
    //    // resize gpu buffer
    //    buffer_sizes_[0] = data_size;
    //    GL_CALL(BufferData, GL_ARRAY_BUFFER, data_size, nullptr, GL_STREAM_DRAW);
    //  }
    //
    //  GL_CALL(BufferSubData, GL_ARRAY_BUFFER, 0, data_size, data);
    //  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, 0);
}

void MTLRenderer::UploadIndexBuffer(void* data, size_t data_size) {
    GPUMTLContext *mtlctx = (GPUMTLContext*)ctx_.get();
    if ([passthroughRenderPipeline_->indexBuffer_ length] >= data_size) {
        memcpy([passthroughRenderPipeline_->indexBuffer_ contents], data, data_size);
    }
    else
    {
        id<MTLBuffer> buf = [mtlctx->mDevice newBufferWithBytes:data length:data_size options:MTLResourceStorageModeShared];
        passthroughRenderPipeline_->indexBuffer_ = buf;
    }
    
    //  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
    //  if (data_size > buffer_sizes_[1]) {
    //    // resize gpu buffer
    //    buffer_sizes_[1] = data_size;
    //    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, data_size, nullptr,
    //            GL_STREAM_DRAW);
    //  }
    //
    //  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, data_size, data);
    //  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MTLRenderer::SetGlobalAlpha(float alpha) {
    passthroughRenderPipeline_->globalAlpha_ = alpha;
    //    shader_->SetGlobalAlpha(alpha);
}

void MTLRenderer::EnableStencilTest() {
    passthroughRenderPipeline_->enableStencilTest_ = true;
    //    GL_CALL(Enable, GL_STENCIL_TEST);
}

void MTLRenderer::DisableStencilTest() {
    passthroughRenderPipeline_->enableStencilTest_ = false;
    //    GL_CALL(Disable, GL_STENCIL_TEST);
}

void MTLRenderer::DisableColorOutput() {
    passthroughRenderPipeline_->writeColor_ = false;
    //    GL_CALL(ColorMask, 0, 0, 0, 0);
}

void MTLRenderer::EnableColorOutput() {
    passthroughRenderPipeline_->writeColor_ = true;
    //    GL_CALL(ColorMask, 1, 1, 1, 1);
}

void MTLRenderer::UpdateStencilMask(uint8_t write_mask) {
    passthroughRenderPipeline_->SetStencilMask(write_mask);
    //  GL_CALL(StencilMask, write_mask);
}

void MTLRenderer::UpdateStencilOp(HWStencilOp op) {
    passthroughRenderPipeline_->SetStencilOp(op);
    //  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, hw_stencil_op_to_gl(op));
}

void MTLRenderer::UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                    uint32_t compare_mask) {
    passthroughRenderPipeline_->SetStencilFunc(func, value, compare_mask);
    //  GL_CALL(StencilFunc, hw_stencil_func_to_gl(func), value, compare_mask);
}

void MTLRenderer::DrawIndex(uint32_t start, uint32_t count) {
    passthroughRenderPipeline_->DrawIndex(start, count);
    //  GL_CALL(DrawElements, GL_TRIANGLES, count, GL_UNSIGNED_INT,
    //          (void*)(start * sizeof(GLuint)));
}

void MTLRenderer::InitShader() {
    //  if (use_gs_) {
    //    shader_ = GLShader::CreateGSPipelineShader();
    //  } else {
    //    shader_ = GLShader::CreatePipelineShader();
    //  }
}

void MTLRenderer::InitBufferObject() {
    //  GL_CALL(GenVertexArrays, 1, &vao_);
    //  GL_CALL(GenBuffers, 2, buffers_.data());
    //
    //  GL_CALL(BindVertexArray, vao_);
    //  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
    //
    //  GL_CALL(EnableVertexAttribArray, 0);
    //  GL_CALL(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
    //          (void*)0);
    //  GL_CALL(EnableVertexAttribArray, 1);
    //  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
    //          (void*)(2 * sizeof(float)));
    //
    //  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
    //
    //  GL_CALL(BindVertexArray, 0);
    //
    //  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER , 0);
    //  GL_CALL(BindBuffer, GL_ARRAY_BUFFER , 0);
}

void MTLRenderer::BindBuffers() {
    //  GL_CALL(BindVertexArray, vao_);
    //  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
}

void MTLRenderer::UnBindBuffers() {
    //  GL_CALL(BindVertexArray, 0);
    //  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MTLRenderer::BindTexture(HWTexture* /* unused */tex, uint32_t slot) {
    if (slot == 0) {
        passthroughRenderPipeline_->texture0_ = tex;
    }
    else if (slot == 1) {
        passthroughRenderPipeline_->texture1_ = tex;
    }
    else
    {
        assert(false);
    }
    //  GL_CALL(ActiveTexture, slot_to_gl_texture_unit(slot));
}

void MTLRenderer::BindRenderTarget(HWRenderTarget* render_target) {
    passthroughRenderPipeline_->renderTarget_ = render_target;
    //  GLRenderTarget* fbo = (GLRenderTarget*)render_target;
    //
    //  fbo->Bind();
    //
    //  // save current viewport
    //  GL_CALL(GetIntegerv, GL_VIEWPORT, &saved_viewport_[0]);
    //
    //  GL_CALL(Viewport, 0, 0, fbo->Width(), fbo->Height());
}

void MTLRenderer::UnBindRenderTarget(HWRenderTarget* render_target) {
    passthroughRenderPipeline_->renderTarget_ = nullptr;
    //  GLRenderTarget* fbo = (GLRenderTarget*)render_target;
    //
    //  fbo->UnBind();
    //
    //  if (root_fbo_ != 0) {
    //    GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, root_fbo_);
    //  }
    //
    //  // restore saved viewport
    //  GL_CALL(Viewport, saved_viewport_[0], saved_viewport_[1], saved_viewport_[2],
    //          saved_viewport_[3]);
}

}  // namespace skity
