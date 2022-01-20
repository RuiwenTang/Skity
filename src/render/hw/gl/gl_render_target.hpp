#ifndef SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP

#include "src/render/hw/hw_render_target.hpp"

namespace skity {

class GLRenderTarget : public HWRenderTarget {
 public:
  GLRenderTarget(std::unique_ptr<HWTexture> color_buffer,
                 std::unique_ptr<HWTexture> stencil_buffer)
      : HWRenderTarget(std::move(color_buffer), std::move(stencil_buffer)),
        fbo_(0) {}
  ~GLRenderTarget() override = default;

  uint32_t GetFrameBufferID() const { return fbo_; }

 protected:
  void OnInit() override;

  void OnDestroy() override;

 private:
  uint32_t fbo_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP