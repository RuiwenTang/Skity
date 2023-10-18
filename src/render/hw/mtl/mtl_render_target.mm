
#include "mtl_render_target.hpp"
#include "src/logging.hpp"
#include "src/render/hw/mtl/mtl_texture.hpp"

namespace skity {

MTLRenderTarget::MTLRenderTarget(uint32_t width, uint32_t height,std::shared_ptr<GPUContext> &gpuContext) : HWRenderTarget(width, height), color_texture_(gpuContext), stencil_texture_(gpuContext), horizontal_texture_(gpuContext),vertical_texture_(gpuContext)
{
    
}

void MTLRenderTarget::Init() {
    
    renderPassDesc_ = [MTLRenderPassDescriptor renderPassDescriptor];
    
//#if defined(__ANDROID__) || defined(ANDROID) || defined(TARGET_OS_IPHONE) ||  defined(TARGET_IPHONE_SIMULATOR)
//  msaa_sample_count_ = 0;
//#else
//  int32_t max_sample_count = 0;
//  GL_CALL(GetIntegerv, GL_MAX_SAMPLES, &max_sample_count);
//  msaa_sample_count_ = std::min((uint32_t)max_sample_count, msaa_sample_count_);
//  // step 1 init all internal textures
//#endif
//
  InitTextures();
//
//  // step 2 bind texture to framebuffer
//  // during init stage, only stencil buffer is bind
//  InitFBO();
}

void MTLRenderTarget::Bind() {
    
}

void MTLRenderTarget::BlitColorTexture() {
//  if (!EnableMultiSample()) {
//    return;
//  }
//#if !defined(__ANDROID__) && !defined(ANDROID) && !defined(TARGET_OS_IPHONE) &&  !defined(TARGET_IPHONE_SIMULATOR)
//  // make sure fbo bound a normal color texture
//  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, fbo_);
//  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//          GL_TEXTURE_2D, color_texture_.GetTextureID(), 0);
//
//  check_fbo_state(fbo_);
//  Clear();
//
//  GL_CALL(BindFramebuffer, GL_READ_FRAMEBUFFER, msaa_fbo_);
//
//  check_fbo_state(msaa_fbo_);
//
//  GL_CALL(BlitFramebuffer, 0, 0, Width(), Height(), 0, 0, Width(), Height(),
//          GL_COLOR_BUFFER_BIT, GL_NEAREST);
//
//  // make sure draw and read fbo bind to normal target
//  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, fbo_);
//#endif
}

void MTLRenderTarget::BindColorTexture() {
    renderPassDesc_.colorAttachments[0].texture = color_texture_.GetInternalTexture();
    renderPassDesc_.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
    renderPassDesc_.colorAttachments[0].loadAction = MTLLoadActionClear;
}

void MTLRenderTarget::BindHorizontalTexture() {
    renderPassDesc_.colorAttachments[0].texture = horizontal_texture_.GetInternalTexture();
    renderPassDesc_.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
    renderPassDesc_.colorAttachments[0].loadAction = MTLLoadActionClear;
}

void MTLRenderTarget::BindVerticalTexture() {
    renderPassDesc_.colorAttachments[0].texture = vertical_texture_.GetInternalTexture();
    renderPassDesc_.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
    renderPassDesc_.colorAttachments[0].loadAction = MTLLoadActionClear;
}

void MTLRenderTarget::UnBind() {

}

void MTLRenderTarget::Destroy() {
//  GL_CALL(DeleteFramebuffers, 1, &fbo_);
  color_texture_.Destroy();
  horizontal_texture_.Destroy();
  vertical_texture_.Destroy();
  stencil_texture_.Destroy();
}

void MTLRenderTarget::InitTextures() {
  // init color texture
  color_texture_.Init(HWTexture::Type::kColorTexture, HWTexture::Format::kRGBA);
  color_texture_.Bind();
  color_texture_.Resize(Width(), Height());

  // init horizontal texture
  horizontal_texture_.Init(HWTexture::Type::kColorTexture,
                           HWTexture::Format::kRGBA);
  horizontal_texture_.Bind();
  horizontal_texture_.Resize(Width(), Height());

  // init vertical texture
  vertical_texture_.Init(HWTexture::Type::kColorTexture,
                         HWTexture::Format::kRGBA);
  vertical_texture_.Bind();
  vertical_texture_.Resize(Width(), Height());


  stencil_texture_.Init(HWTexture::Type::kStencilTexture,
                        HWTexture::Format::kS);
  stencil_texture_.Bind();
  stencil_texture_.Resize(Width(), Height());

  stencil_texture_.UnBind();

}

void MTLRenderTarget::InitFBO() {
    
    renderPassDesc_.stencilAttachment.texture = stencil_texture_.GetInternalTexture();
    renderPassDesc_.stencilAttachment.clearStencil = 0;
    renderPassDesc_.stencilAttachment.loadAction = MTLLoadActionClear;
    
//  GL_CALL(GenFramebuffers, 1, &fbo_);
//
//  if (fbo_ == 0) {
//    LOG_ERROR("Failed to create gl frame buffer object!!");
//    return;
//  }
//
//  if (!EnableMultiSample()) {
//    GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, fbo_);
//
//#if defined(__ANDROID__) || defined(ANDROID) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
//    // bind stencil attachment
//    GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
//            GL_TEXTURE_2D, stencil_texture_.GetTextureID(), 0);
//#else
//    // bind stencil attachment
//    GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
//            GL_TEXTURE_2D, stencil_texture_.GetTextureID(), 0);
//#endif
//
//    check_fbo_state(fbo_);
//    GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, 0);
//
//    return;
//  }
//
//  // init a msaa fbo
//  GL_CALL(GenFramebuffers, 1, &msaa_fbo_);
//  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, msaa_fbo_);
//  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//          GL_TEXTURE_2D_MULTISAMPLE, msaa_target_, 0);
//
//#if defined(__ANDROID__) || defined(ANDROID) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
//  // bind stencil attachment
//  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
//          GL_TEXTURE_2D_MULTISAMPLE, stencil_texture_.GetTextureID(), 0);
//#else
//  // bind stencil attachment
//  GL_CALL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
//          GL_TEXTURE_2D_MULTISAMPLE, stencil_texture_.GetTextureID(), 0);
//#endif
//
//
//  check_fbo_state(msaa_fbo_);
//  GL_CALL(BindFramebuffer, GL_FRAMEBUFFER, 0);
}

void MTLRenderTarget::InitMultiSampleTexture() {
//#if !defined(__ANDROID__) && !defined(ANDROID) && !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
//  GL_CALL(GenTextures, 1, &msaa_target_);
//
//  GL_CALL(BindTexture, GL_TEXTURE_2D_MULTISAMPLE, msaa_target_);
//
//  GL_CALL(TexImage2DMultisample, GL_TEXTURE_2D_MULTISAMPLE, msaa_sample_count_,
//          color_texture_.GetInternalFormat(), Width(), Height(), GL_TRUE);
//
//  GL_CALL(BindTexture, GL_TEXTURE_2D_MULTISAMPLE, 0);
//#endif
}

void MTLRenderTarget::Clear() {
//#if !defined(__ANDROID__) && !defined(ANDROID) && !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
//  static const GLenum color_buffer = GL_COLOR_ATTACHMENT0;
//  static const GLenum stencil_buffer = GL_DEPTH_STENCIL_ATTACHMENT;
//
//  static const float transparent[] = {0.f, 0.f, 0.f, 0.f};
//  static const int32_t zero = 0;
//
//  // clear color
//  GL_CALL(DrawBuffers, 1, &color_buffer);
//  GL_CALL(ClearBufferfv, GL_COLOR, 0, transparent);
//
//  // clear stencil
//  GL_CALL(ClearBufferfi, GL_STENCIL, 0, 0.f, 0);
//#else
//  GL_CALL(ClearColor,0,0,0,0);
//  GL_CALL(ClearStencil,0);
//  GL_CALL(Clear,GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//#endif
}

}  // namespace skity
