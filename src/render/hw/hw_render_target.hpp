#ifndef SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP
#define SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include "src/render/hw/hw_texture.hpp"

namespace skity {

// Need to call HWPipelne::BindRenderTarget first,
// otherwise, all Bindxxx function is not working
class HWRenderTarget {
 public:
  HWRenderTarget(uint32_t width, uint32_t height)
      : width_(width), height_(height) {}
  virtual ~HWRenderTarget() = default;

  uint32_t Width() const { return width_; }
  uint32_t Height() const { return height_; }

  virtual HWTexture* ColorTexture() = 0;

  virtual HWTexture* HorizontalTexture() = 0;

  virtual HWTexture* VerticalTexture() = 0;

  virtual void BindColorTexture() = 0;

  virtual void BindHorizontalTexture() = 0;

  virtual void BindVerticalTexture() = 0;

  virtual void Init() = 0;

  virtual void Destroy() = 0;

 private:
  uint32_t width_ = 0;
  uint32_t height_ = 0;
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

  void EndFrame();

  void CleanUp();

 private:
  void ClearUsedFlags();

 private:
  std::unordered_map<HWRenderTarget*, RefRenderTarget> target_cache_ = {};
  std::unordered_map<Size, std::vector<Info>, HWRenderTargetCache::SizeHash>
      info_map_ = {};
  size_t current_age_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_RENDER_TARGET_HPP