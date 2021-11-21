#ifndef SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP
#define SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP

#include <skity/graphic/path.hpp>
#include <vector>

namespace skity {

class HWCanvasState {
 public:
  HWCanvasState();
  ~HWCanvasState() = default;

  void Save();
  void Restore();
  void Translate(float dx, float dy);
  void Scale(float dx, float dy);
  void Rotate(float degree);
  void Rotate(float degree, float px, float py);
  void Concat(Matrix const& matrix);

  Matrix CurrentMatrix();

  bool HasClip();

  bool MatrixDirty();
  void ClearMatrixDirty();

 private:
  void PushMatrixStack();
  void PopMatrixStack();

 private:
  std::vector<Matrix> matrix_state_ = {};
  bool matrix_dirty_ = true;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP