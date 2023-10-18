
#ifndef mtl_texture_hpp
#define mtl_texture_hpp

#include "src/render/hw/hw_texture.hpp"
#include "skity/gpu/gpu_mtl_context.hpp"
#include <memory>
#import <Metal/Metal.h>

namespace skity {
    
    class GPUContext;
    class MTLTexture : public HWTexture {
    public:
        MTLTexture(std::shared_ptr<GPUContext> &gpuContext);
        ~MTLTexture() override;
        
        void SetMultisample(uint32_t msaa_count) { msaa_count_ = msaa_count; }
        
        void Init(HWTexture::Type type, HWTexture::Format format) override;
        
        void Destroy() override;
        
        void Bind() override;
        void UnBind() override;
        
        uint32_t GetWidth() override;
        uint32_t GetHeight() override;
        
        void Resize(uint32_t width, uint32_t height) override;
        
        void UploadData(uint32_t offset_x, uint32_t offset_y, uint32_t width,
                        uint32_t height,size_t row_bytes, void* data) override;
        
        id<MTLTexture> GetInternalTexture(){
            return mtlTexture_;
        }
        
    private:
        uint32_t msaa_count_ = 0;
        std::shared_ptr<GPUContext> gpuContext_;
        HWTexture::Format format_;
        __strong id<MTLTexture> mtlTexture_;
    };
    
}  // namespace skity

#endif /* mtl_texture_hpp */
