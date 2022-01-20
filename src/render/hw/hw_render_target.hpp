#ifndef SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP

#include <memory>

#include "src/render/hw/hw_texture.hpp"

namespace skity {

class HWRenderTarget {
 public:
  HWRenderTarget(std::unique_ptr<HWTexture> color_buffer,
                 std::unique_ptr<HWTexture> stencil_buffer)
      : c_buffer_(std::move(color_buffer)),
        s_buffer_(std::move(stencil_buffer)) {}
  virtual ~HWRenderTarget() = default;

  HWTexture* ColorBuffer() const { return c_buffer_.get(); }

  HWTexture* StencilBuffer() const { return s_buffer_.get(); }

  void Init() { OnInit(); }

  void Destroy() {
    OnDestroy();
    c_buffer_->Destroy();
    s_buffer_->Destroy();
  }

 protected:
  virtual void OnInit() = 0;
  virtual void OnDestroy() = 0;

 private:
  std::unique_ptr<HWTexture> c_buffer_;
  std::unique_ptr<HWTexture> s_buffer_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP