#ifndef SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP
#define SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP

#include <functional>
#include <skity/graphic/path.hpp>
#include <vector>

#include "src/render/hw/hw_draw.hpp"

namespace skity {

class HWCanvasState {
 public:
  struct ClipStackValue {
    uint32_t stack_depth = {};
    HWDrawRange front_range = {};
    HWDrawRange back_range = {};
    HWDrawRange bound_range = {};
    Matrix stack_matrix = {};
  };

  HWCanvasState();
  ~HWCanvasState() = default;

  void Save();
  void Restore();
  void Translate(float dx, float dy);
  void Scale(float dx, float dy);
  void Rotate(float degree);
  void Rotate(float degree, float px, float py);
  void Concat(Matrix const& matrix);

  void SaveClipPath(HWDrawRange const& front_range,
                    HWDrawRange const& back_range,
                    HWDrawRange const& bound_range, Matrix const& matrix);

  bool ClipStackEmpty();

  bool NeedRevertClipStencil();

  ClipStackValue CurrentClipStackValue();

  void ForEachClipStackValue(
      std::function<void(ClipStackValue const&, size_t)> const& func);

  Matrix CurrentMatrix();

  bool HasClip();

  bool MatrixDirty();
  void ClearMatrixDirty();

 private:
  void PushMatrixStack();
  void PopMatrixStack();
  void PopClipStack();

 private:
  std::vector<Matrix> matrix_state_ = {};
  std::vector<ClipStackValue> clip_stack_ = {};
  bool matrix_dirty_ = true;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP