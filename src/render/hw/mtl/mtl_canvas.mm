
#include "mtl_canvas.hpp"
#include "src/render/hw/mtl/mtl_render_target.hpp"
#include "src/render/hw/mtl/mtl_texture.hpp"
#include "mtl_renderer.hpp"

namespace skity {
    
    MTLCanvas::MTLCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : HWCanvas(mvp, width, height, density) {
        
    }
    
    void MTLCanvas::OnInit(std::shared_ptr<GPUContext>& ctx) {
        ctx_ = ctx;
    }
    
    
    bool MTLCanvas::SupportGeometryShader() {
        return false;
    }
    
    std::unique_ptr<HWRenderer> MTLCanvas::CreateRenderer() {
        auto renderer = std::make_unique<MTLRenderer>(ctx_, SupportGeometryShader());
        renderer->Init();

        return renderer;
    }
    
    std::unique_ptr<HWTexture> MTLCanvas::GenerateTexture() {
        return std::make_unique<MTLTexture>(ctx_);
    }
    
    std::unique_ptr<HWFontTexture> MTLCanvas::GenerateFontTexture(Typeface* typeface) {
        //  return std::make_unique<GLFontTexture>(typeface);
        assert(false);
        return nullptr;
    }
    
    std::unique_ptr<HWRenderTarget> MTLCanvas::GenerateBackendRenderTarget(uint32_t width, uint32_t height) {
        auto fbo = std::make_unique<MTLRenderTarget>(width, height,ctx_);
        
        fbo->SetEnableMultiSample(false);
        
        fbo->Init();
        
        return fbo;
        
    }
    
}  // namespace skity
