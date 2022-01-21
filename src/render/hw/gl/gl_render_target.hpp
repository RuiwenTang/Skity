#ifndef SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_GL_RENDER_TARGET_HPP

#include "src/render/hw/hw_render_target.hpp"

namespace skity {

class GLRenderTarget : public HWRenderTarget {
 public:
  GLRenderTarget(std::unique_ptr<HWTexture> hcolor_buffer,
                 std::unique_ptr<HWTexture> vcolor_buffer,
                 std::unique_ptr<HWTexture> stencil_buffer)
      : HWRenderTarget(std::move(hcolor_buffer), std::move(vcolor_buffer),
                       std::move(stencil_buffer)),
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