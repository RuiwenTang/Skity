
#ifndef gpu_mtl_context_hpp
#define gpu_mtl_context_hpp

#include <skity/config.hpp>
#include <skity/gpu/gpu_context.hpp>
#import <Metal/Metal.h>

namespace skity {
    struct GPUMTLContext : public GPUContext{
    public:
        using GPUContext::GPUContext;
        __strong id<MTLDevice> mDevice;
        __strong id<MTLCommandQueue> mCommandQueue;
        __strong id <MTLLibrary> mDefaultLibrray;
        
        __strong id<MTLTexture> mColorTexture;
        __strong id<MTLTexture> mStencilTexture;
    };
}

#endif /* gpu_mtl_context_hpp */
