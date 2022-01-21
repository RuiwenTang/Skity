#ifndef SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP

#include <memory>
#include <unordered_map>
#include <vector>

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

  uint32_t Width() const { return c_buffer_->GetWidth(); }

  uint32_t Height() const { return c_buffer_->GetHeight(); }

 protected:
  virtual void OnInit() = 0;
  virtual void OnDestroy() = 0;

 private:
  std::unique_ptr<HWTexture> c_buffer_;
  std::unique_ptr<HWTexture> s_buffer_;
};

class HWRenderTargetCache final {
  using RefRenderTarget = std::unique_ptr<HWRenderTarget>;

 public:
  struct Size {
    uint32_t width = {};
    uint32_t height = {};

    Size() = default;

    bool operator==(Size const& other) const {
      return width == other.width && height == other.height;
    }
  };

  struct SizeHash {
    std::size_t operator()(Size const& size) const {
      size_t res = 17;

      res = res * 31 + std::hash<uint32_t>()(size.width);
      res = res * 31 + std::hash<uint32_t>()(size.height);

      return res;
    }
  };

  struct Info {
    size_t age = {};
    bool used = false;
    HWRenderTarget* target = {};
  };

  HWRenderTargetCache() = default;
  ~HWRenderTargetCache() = default;

  HWRenderTarget* QueryTarget(uint32_t width, uint32_t height);

  HWRenderTarget* StoreCache(std::unique_ptr<HWRenderTarget> target);

  void BeginFrame();

 private:
  std::unordered_map<HWRenderTarget*, RefRenderTarget> target_cache_ = {};
  std::unordered_map<Size, std::vector<Info>, HWRenderTargetCache::SizeHash>
      info_map_ = {};
  size_t current_age_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP