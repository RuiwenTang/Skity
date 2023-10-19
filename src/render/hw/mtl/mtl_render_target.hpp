
#ifndef mtl_render_target_hpp
#define mtl_render_target_hpp

#include <memory>
#include "src/render/hw/mtl/mtl_texture.hpp"
#include "src/render/hw/hw_render_target.hpp"

namespace skity {

class GPUContext;
class MTLRenderTarget : public HWRenderTarget {
public:
    MTLRenderTarget(uint32_t width, uint32_t height,std::shared_ptr<GPUContext> &gpuContext);
    ~MTLRenderTarget() override = default;
    
    HWTexture* ColorTexture() override { return &color_texture_; }
    
    HWTexture* HorizontalTexture() override { return &horizontal_texture_; }
    
    HWTexture* VerticalTexture() override { return &vertical_texture_; }
    
    HWTexture* StencilTexture(){return &stencil_texture_;}
    
    void BindColorTexture() override;
    
    void BlitColorTexture() override;
    
    void BindHorizontalTexture() override;
    
    void BindVerticalTexture() override;
    
    void Init() override;
    
    void Destroy() override;
    
    void Bind();
    
    void UnBind();
    
    MTLRenderPassDescriptor *GetRenderPass(){
        return renderPassDesc_;
    }
    
private:
    void InitTextures();
    void InitFBO();
    void InitMultiSampleTexture();
    
    void Clear();
    
private:
    MTLTexture color_texture_;
    MTLTexture horizontal_texture_;
    MTLTexture vertical_texture_;
    MTLTexture stencil_texture_;
    __strong MTLRenderPassDescriptor *renderPassDesc_;
};
}

#endif /* mtl_render_target_hpp */
