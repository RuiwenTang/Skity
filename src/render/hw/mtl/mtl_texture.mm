
#include "mtl_texture.hpp"
#include "skity/gpu/gpu_mtl_context.hpp"
#include "mtl_passthrough_pipeline.hpp"
#import <Metal/Metal.h>

namespace skity {
    
    static MTLPixelFormat hw_texture_format_to_mtl(HWTexture::Format format) {
        switch (format) {
            case HWTexture::Format::kR:
                return MTLPixelFormatR8Unorm;
            case HWTexture::Format::kRGB:
                assert(false);
                return MTLPixelFormatRGBA8Unorm;
            case HWTexture::Format::kRGBA:
                return MTLPixelFormatRGBA8Unorm;
            case HWTexture::Format::kS:
                return MTLPixelFormatStencil8;
        }
        
        return MTLPixelFormatRGBA8Unorm;
    }
    
    
    MTLTexture::MTLTexture(std::shared_ptr<GPUContext> &gpuContext){
        gpuContext_ = gpuContext;
    }
    
    MTLTexture::~MTLTexture() {

    }
    
    void MTLTexture::Init(HWTexture::Type type, HWTexture::Format format) {
        
        format_ = format;
    }
    
    void MTLTexture::Destroy() {
        mtlTexture_ = nil;
    }
    
    void MTLTexture::Bind() {
        //  if (msaa_count_ > 0) {
        //    GL_CALL(BindTexture, GL_TEXTURE_2D_MULTISAMPLE, texture_id_);
        //  } else {
        //    GL_CALL(BindTexture, GL_TEXTURE_2D, texture_id_);
        //  }
    }
    
    void MTLTexture::UnBind() {
        //  if (msaa_count_) {
        //
        //    GL_CALL(BindTexture, GL_TEXTURE_2D_MULTISAMPLE, 0);
        //  } else {
        //    GL_CALL(BindTexture, GL_TEXTURE_2D, 0);
        //  }
    }
    
    uint32_t MTLTexture::GetWidth() { return [mtlTexture_ width]; }
    
    uint32_t MTLTexture::GetHeight() { return [mtlTexture_ height]; }
    
    void MTLTexture::Resize(uint32_t width, uint32_t height) {
        if (width > 0 && height > 0) {
            GPUMTLContext *mtlContext = (GPUMTLContext*)gpuContext_.get();
            MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:hw_texture_format_to_mtl(format_) width:width height:height mipmapped:NO];
            mtlTexture_ = [mtlContext->mDevice newTextureWithDescriptor:desc];
        }
    }
    
    void MTLTexture::UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                                uint32_t height, size_t row_bytes,void *data) {
        [mtlTexture_ replaceRegion:MTLRegionMake2D(offset_x, offset_y, width, height) mipmapLevel:0 withBytes:data bytesPerRow:row_bytes];
    }
    
}  // namespace skity
