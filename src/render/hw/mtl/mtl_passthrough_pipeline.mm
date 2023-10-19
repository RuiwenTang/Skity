
#include "mtl_passthrough_pipeline.hpp"
#include "skity/gpu/gpu_mtl_context.hpp"
#include "mtl_render_target.hpp"
#include "mtl_texture.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace skity
{

MTLPassthroughRenderPipeline::~MTLPassthroughRenderPipeline()
{
    
}

MTLPassthroughRenderPipeline::MTLPassthroughRenderPipeline(std::shared_ptr<GPUContext> &ctx)
{
    ctx_ = ctx;
    
    GPUMTLContext *mtlctx = (GPUMTLContext*)ctx_.get();
    
    auto CreatPipelineFun = [&](bool needStencil)->id<MTLRenderPipelineState>{
        MTLVertexDescriptor *mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];

        // Positions.
        mtlVertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        mtlVertexDescriptor.attributes[0].offset = 0;
        mtlVertexDescriptor.attributes[0].bufferIndex = 0;

        // Texture coordinates.
        mtlVertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
        mtlVertexDescriptor.attributes[1].offset = 2 * 4;
        mtlVertexDescriptor.attributes[1].bufferIndex = 0;


        // Position buffer layout.
        mtlVertexDescriptor.layouts[0].stride = 4 * 5;
        mtlVertexDescriptor.layouts[0].stepRate = 1;
        mtlVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    //    // Generic attribute buffer layout.
    //    mtlVertexDescriptor.layouts[1].stride = 20;
    //    mtlVertexDescriptor.layouts[1].stepRate = 1;
    //    mtlVertexDescriptor.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;
        
        MTLRenderPipelineDescriptor *des = [[MTLRenderPipelineDescriptor alloc] init];
        des.rasterSampleCount = 1;
        des.vertexDescriptor = mtlVertexDescriptor;
        des.vertexFunction = [mtlctx->mDefaultLibrray newFunctionWithName:@"passThroughVertex"];
        des.fragmentFunction = [mtlctx->mDefaultLibrray newFunctionWithName:@"passThroughtFragment"];
        if (needStencil) {
            des.stencilAttachmentPixelFormat = MTLPixelFormatStencil8;
        }
        des.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        des.colorAttachments[0].blendingEnabled = YES;
        des.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        des.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        des.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        des.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    //    des.colorAttachments[0].writeMask = MTLColorWriteMask
        NSError *error = nil;
        id<MTLRenderPipelineState> pipeline = [mtlctx->mDevice newRenderPipelineStateWithDescriptor:des error:&error];
        assert(error == nil);
        return pipeline;
    };
    
    renderPipelineWithStencil_ = CreatPipelineFun(true);
    placeHolderBuffer_ = [mtlctx->mDevice newBufferWithLength:1024 options:MTLResourceStorageModePrivate];
}


void MTLPassthroughRenderPipeline::SetStencilOp(HWStencilOp op)
{
    if (stencilOp_ != op) {
        stencilOp_ = op;
        stencilStateDirty_ = true;
    }
}

void MTLPassthroughRenderPipeline::SetStencilFunc(HWStencilFunc func, uint32_t value, uint32_t compare_mask )
{
    if (stencilFun_ != func || stencilCompareMask_ != compare_mask) {
        stencilFun_ = func;
        stencilCompareMask_ = compare_mask;
        stencilStateDirty_ = true;
    }
    stencilRefValue_ = value;
}

void MTLPassthroughRenderPipeline::SetStencilMask(uint8_t write_mask)
{
    if (stencilWriteMask_ != write_mask) {
        stencilWriteMask_ = write_mask;
        stencilStateDirty_ = true;
    }
}

void MTLPassthroughRenderPipeline::DrawIndex(uint32_t start, uint32_t count)
{
    GPUMTLContext *mtlctx = (GPUMTLContext*)ctx_.get();
    int width , height;
    
    MTLRenderPassDescriptor *renderPassDesc = nil;
    if (renderTarget_) {
        MTLRenderTarget *mtlTarget = (MTLRenderTarget*)renderTarget_;
        renderPassDesc = mtlTarget->GetRenderPass();
        width = mtlTarget->Width();
        height = mtlTarget->Height();
    }
    else
    {
        renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
        renderPassDesc.colorAttachments[0].texture = mtlctx->mColorTexture;
        //if (enableStencilTest_) {
            renderPassDesc.stencilAttachment.texture = mtlctx->mStencilTexture;
            renderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
            renderPassDesc.stencilAttachment.storeAction = MTLStoreActionStore;
            renderPassDesc.stencilAttachment.clearStencil = 0;
        //}
        
        
        width = mtlctx->mColorTexture.width;
        height = mtlctx->mColorTexture.height;
    }
    
    if (writeColor_) {
        renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    }
    else
    {
        renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionDontCare;
    }
    
    id<MTLCommandBuffer> cmdbuffer = [mtlctx->mCommandQueue commandBuffer];

    id<MTLRenderCommandEncoder> renderEncoder = [cmdbuffer renderCommandEncoderWithDescriptor:renderPassDesc];
    [renderEncoder setViewport:(MTLViewport){0.0, 0.0, (double)width, (double)height, -1.0, 1.0 }];
    //if (enableStencilTest_)
    {
        [renderEncoder setRenderPipelineState:renderPipelineWithStencil_];
    }
    //else
    {
//        [renderEncoder setRenderPipelineState:renderPipeline_];
    }
    
    [renderEncoder setVertexBuffer:vertexBuffer_ offset:0 atIndex:0];
    [renderEncoder setVertexBytes:glm::value_ptr(mvp_) length:sizeof(mvp_) atIndex:1];
    [renderEncoder setVertexBytes:glm::value_ptr(modelMat_) length:sizeof(modelMat_) atIndex:2];
    [renderEncoder setStencilReferenceValue:stencilRefValue_];
    if (enableStencilTest_) {
        [renderEncoder setDepthStencilState:CreateStencilState()];
    }
    
    if (texture0_) {
        [renderEncoder setFragmentTexture:((MTLTexture*)texture0_)->GetInternalTexture() atIndex:0];
    }
    if (texture1_) {
        [renderEncoder setFragmentTexture:((MTLTexture*)texture1_)->GetInternalTexture() atIndex:1];
    }
    
    [renderEncoder setFragmentBytes:glm::value_ptr(modelMat_) length:sizeof(modelMat_) atIndex:0];
    [renderEncoder setFragmentBytes:&globalAlpha_ length:sizeof(globalAlpha_) atIndex:1];
    [renderEncoder setFragmentBytes:glm::value_ptr(uniform_color_) length:sizeof(uniform_color_) atIndex:2];
    [renderEncoder setFragmentBytes:&strokeWidth_ length:sizeof(strokeWidth_) atIndex:3];
    [renderEncoder setFragmentBytes:&color_type length:sizeof(color_type) atIndex:4];
    if (!gradientColors_.empty()) {
        int counts[2] = {static_cast<int>(gradientColors_.size()),static_cast<int>(gradientPos_.size())};
        [renderEncoder setFragmentBytes:&counts length:sizeof(counts) atIndex:5];
        [renderEncoder setFragmentBytes:glm::value_ptr(gradientBoundInfo_) length:sizeof(gradientBoundInfo_) atIndex:6];
        [renderEncoder setFragmentBytes:gradientColors_.data() length:gradientColors_.size() * sizeof(glm::vec4) atIndex:7];
        [renderEncoder setFragmentBytes:gradientPos_.data() length:gradientPos_.size() * sizeof(float) atIndex:8];
    }
    else
    {
        [renderEncoder setFragmentBuffer:placeHolderBuffer_ offset:0 atIndex:5];
        [renderEncoder setFragmentBuffer:placeHolderBuffer_ offset:0 atIndex:6];
        [renderEncoder setFragmentBuffer:placeHolderBuffer_ offset:0 atIndex:7];
        [renderEncoder setFragmentBuffer:placeHolderBuffer_ offset:0 atIndex:8];
    }
    
    
    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count indexType:MTLIndexTypeUInt32 indexBuffer:indexBuffer_ indexBufferOffset:start * 4];

    [renderEncoder endEncoding];
    
    [cmdbuffer commit];
}


#pragma mark - private

id<MTLDepthStencilState> MTLPassthroughRenderPipeline::CreateStencilState()
{
    if (stencilStateDirty_) {
        stencilStateDirty_ = false;
        GPUMTLContext *mtlctx = (GPUMTLContext*)ctx_.get();
        MTLDepthStencilDescriptor *depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
        MTLStencilDescriptor *stencilDesc = [MTLStencilDescriptor new];
        stencilDesc.readMask = stencilCompareMask_;
        stencilDesc.writeMask = stencilWriteMask_;
        
        switch (stencilFun_) {
            case HWStencilFunc::ALWAYS:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionAlways;
                break;
            case HWStencilFunc::EQUAL:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionEqual;
                break;
            case HWStencilFunc::NOT_EQUAL:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionNotEqual;
                break;
            case HWStencilFunc::LESS:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionLess;
                break;
            case HWStencilFunc::GREAT:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionGreater;
                break;
            case HWStencilFunc::LESS_OR_EQUAL:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionLessEqual;
                break;
            case HWStencilFunc::GREAT_OR_EQUAL:
                stencilDesc.stencilCompareFunction = MTLCompareFunctionGreaterEqual;
                break;

            default:
                assert(false);
                break;
        }
        
        stencilDesc.stencilFailureOperation = MTLStencilOperationKeep;
        stencilDesc.depthFailureOperation = MTLStencilOperationKeep;
        switch (stencilOp_) {
            case HWStencilOp::KEEP:
                stencilDesc.depthStencilPassOperation = MTLStencilOperationKeep;
                break;
            case HWStencilOp::INCR_WRAP:
                stencilDesc.depthStencilPassOperation = MTLStencilOperationIncrementWrap;
                break;
            case HWStencilOp::DECR_WRAP:
                stencilDesc.depthStencilPassOperation = MTLStencilOperationDecrementWrap;
                break;
            case HWStencilOp::REPLACE:
                stencilDesc.depthStencilPassOperation = MTLStencilOperationReplace;
                break;

            default:
                assert(false);
                break;
        }
        
        depthStencilDesc.backFaceStencil = stencilDesc;
        depthStencilDesc.frontFaceStencil = stencilDesc;
        
        depthstencilState_ = [mtlctx->mDevice newDepthStencilStateWithDescriptor:depthStencilDesc];
    }
    return depthstencilState_;
}

}
