
#ifndef mtl_canvas_hpp
#define mtl_canvas_hpp

#include <memory>
#include "src/render/hw/hw_renderer.hpp"
#include "src/render/hw/hw_canvas.hpp"

namespace skity {
    
    class MTLCanvas : public HWCanvas {
    public:
        MTLCanvas(Matrix mvp, uint32_t width, uint32_t height, float density);
        ~MTLCanvas() override = default;
        
    protected:
        void OnInit(std::shared_ptr<GPUContext>& ctx) override;
        bool SupportGeometryShader() override;
        std::unique_ptr<HWRenderer> CreateRenderer() override;
        std::unique_ptr<HWTexture> GenerateTexture() override;
        std::unique_ptr<HWFontTexture> GenerateFontTexture(
                                                           Typeface* typeface) override;
        std::unique_ptr<HWRenderTarget> GenerateBackendRenderTarget(
                                                                    uint32_t width, uint32_t height) override;
        
    private:
        std::shared_ptr<GPUContext> ctx_;
//        MTLRenderer* mtl_renderer_;
    };
    
}  // namespace skity


#endif /* mtl_canvas_hpp */
