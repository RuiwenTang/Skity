
#ifndef mtl_passthrough_pipeline_hpp
#define mtl_passthrough_pipeline_hpp

#import <Metal/Metal.h>
#include <memory>
#include "src/render/hw/hw_renderer.hpp"

namespace skity {

class GPUContext;
class MTLPassthroughRenderPipeline{
public:
    virtual ~MTLPassthroughRenderPipeline();
    MTLPassthroughRenderPipeline(std::shared_ptr<GPUContext> &ctx);
    
    void DrawIndex(uint32_t start, uint32_t count);
    
    void SetStencilMask(uint8_t write_mask) ;

    void SetStencilOp(HWStencilOp op) ;

    void SetStencilFunc(HWStencilFunc func, uint32_t value,
                           uint32_t compare_mask);
        
public:
    id<MTLRenderPipelineState> renderPipelineWithStencil_;
    std::shared_ptr<GPUContext> ctx_;
    
    id<MTLBuffer> vertexBuffer_;
    id<MTLBuffer> indexBuffer_;
    float strokeWidth_ = 1;
    glm::vec4 uniform_color_;
    float globalAlpha_ = 1;
    bool writeColor_ = true;
    int32_t color_type = 0;
    
    glm::mat4 mvp_ = glm::mat4(1.0);
    glm::mat4 modelMat_ = glm::mat4(1.0);;
    
    HWRenderTarget *renderTarget_ = nullptr;
    HWTexture *texture0_ = nullptr;
    HWTexture *texture1_ = nullptr;
    
    bool enableStencilTest_ = false;
    
    glm::vec4 gradientBoundInfo_;
    std::vector<glm::vec4> gradientColors_;
    std::vector<float> gradientPos_;
    
private:
    id<MTLDepthStencilState> CreateStencilState();
    id<MTLBuffer> placeHolderBuffer_;
    id<MTLDepthStencilState> depthstencilState_;
    int32_t stencilWriteMask_ = 0xFF;
    HWStencilOp stencilOp_ = HWStencilOp::KEEP;
    HWStencilFunc stencilFun_ = HWStencilFunc::ALWAYS;
    uint32_t stencilRefValue_ = 0;
    uint32_t stencilCompareMask_ = 0xFF;
    bool stencilStateDirty_ = true;
};

}


#endif /* mtl_passthrough_pipeline_hpp */
