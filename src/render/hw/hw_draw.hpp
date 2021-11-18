#ifndef SKITY_SRC_RENDER_HW_HW_DRAW_HPP
#define SKITY_SRC_RENDER_HW_HW_DRAW_HPP

#include <cstdint>

namespace skity {

enum class HWDrawType {
  STENCIL_FRONT,
  STENCIL_BACK,
  COLOR,
  AA,
};

struct HWDrawRange {
  uint32_t start = 0;
  uint32_t count = 0;
};

class HWDraw {
 public:
  HWDraw(HWDrawType type);
  virtual ~HWDraw() = default;

  void Draw(bool has_clip);

  HWDrawType DrawType() const { return type_; }

 protected:
  virtual void OnDraw(bool has_clip) = 0;

 private:
  HWDrawType type_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_DRAW_HPP