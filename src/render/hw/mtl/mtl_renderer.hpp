
#ifndef mtl_renderer_hpp
#define mtl_renderer_hpp

#include <array>
#include <memory>
#include <skity/gpu/gpu_context.hpp>
#include "mtl_passthrough_pipeline.hpp"
#include "src/render/hw/hw_renderer.hpp"

namespace skity {

class GPUContext;
class MTLRenderer : public HWRenderer {
 public:
    MTLRenderer(std::shared_ptr<GPUContext> &ctx, bool use_gs);
  ~MTLRenderer() override;

  void Init() override;

  void Destroy() override;

  void Bind() override;

  void UnBind() override;

  void SetViewProjectionMatrix(glm::mat4 const& mvp) override;

  void SetModelMatrix(glm::mat4 const& matrix) override;

  void SetPipelineColorMode(HWPipelineColorMode mode) override;

  void SetStrokeWidth(float width) override;

  void SetUniformColor(glm::vec4 const& color) override;

  void SetGradientBoundInfo(glm::vec4 const& info) override;

  void SetGradientCountInfo(int32_t color_count, int32_t pos_count) override;

  void SetGradientColors(std::vector<Color4f> const& colors) override;

  void SetGradientPositions(std::vector<float> const& pos) override;

  void UploadVertexBuffer(void* data, size_t data_size) override;

  void UploadIndexBuffer(void* data, size_t data_size) override;

  void SetGlobalAlpha(float alpha) override;

  void EnableStencilTest() override;

  void DisableStencilTest() override;

  void EnableColorOutput() override;

  void DisableColorOutput() override;

  void UpdateStencilMask(uint8_t write_mask) override;

  void UpdateStencilOp(HWStencilOp op) override;

  void UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                         uint32_t compare_mask) override;

  void DrawIndex(uint32_t start, uint32_t count) override;

  void BindTexture(HWTexture* texture, uint32_t slot) override;

  void BindRenderTarget(HWRenderTarget* render_target) override;

  void UnBindRenderTarget(HWRenderTarget* render_target) override;

 private:
  void InitShader();
  void InitBufferObject();

  void BindBuffers();
  void UnBindBuffers();

 private:
    std::unique_ptr<MTLPassthroughRenderPipeline> passthroughRenderPipeline_;
    std::shared_ptr<GPUContext> ctx_;
};

}  // namespace skity


#endif /* mtl_renderer_hpp */
