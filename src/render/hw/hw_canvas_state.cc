#include "src/render/hw/hw_canvas_state.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace skity {

HWCanvasState::HWCanvasState() {
  // init first stack matrix
  matrix_state_.emplace_back(glm::identity<Matrix>());
}

void HWCanvasState::Save() { PushMatrixStack(); }

void HWCanvasState::Restore() {
  Matrix prev_matrix = CurrentMatrix();
  PopMatrixStack();
  PopClipStack();

  // matrix_dirty_ = CurrentMatrix() != prev_matrix;
  matrix_dirty_ = true;
}

void HWCanvasState::Translate(float dx, float dy) {
  Matrix current = CurrentMatrix();
  Matrix translate = glm::translate(glm::identity<Matrix>(), {dx, dy, 0.f});

  matrix_state_.back() = current * translate;

  matrix_dirty_ = true;
}

void HWCanvasState::Scale(float dx, float dy) {
  Matrix current = CurrentMatrix();
  Matrix scale = glm::scale(glm::identity<Matrix>(), {dx, dy, 1.f});

  matrix_state_.back() = current * scale;

  matrix_dirty_ = true;
}

void HWCanvasState::Rotate(float degree) {
  Matrix current = CurrentMatrix();
  Matrix rotate = glm::rotate(glm::identity<Matrix>(), glm::radians(degree),
                              {0.f, 0.f, 1.f});
  matrix_state_.back() = current * rotate;

  matrix_dirty_ = true;
}

void HWCanvasState::Rotate(float degree, float px, float py) {
  Matrix current = CurrentMatrix();
  Matrix rotate = glm::rotate(glm::identity<Matrix>(), glm::radians(degree),
                              {0.f, 0.f, 1.f});
  Matrix pre = glm::translate(glm::identity<Matrix>(), {-px, -py, 0.f});
  Matrix post = glm::translate(glm::identity<Matrix>(), {px, py, 0.f});

  matrix_state_.back() = current * post * rotate * pre;

  matrix_dirty_ = true;
}

void HWCanvasState::Concat(const Matrix &matrix) {
  Matrix current = CurrentMatrix();

  matrix_state_.back() = current * matrix;

  matrix_dirty_ = true;
}

void HWCanvasState::SaveClipPath(HWDrawRange const &front_range,
                                 HWDrawRange const &back_range,
                                 HWDrawRange const &bound_range,
                                 Matrix const &matrix) {
  ClipStackValue value{};
  value.stack_depth = matrix_state_.size();
  value.front_range = front_range;
  value.back_range = back_range;
  value.bound_range = bound_range;
  value.stack_matrix = matrix;

  if (clip_stack_.empty() ||
      clip_stack_.back().stack_depth < value.stack_depth) {
    clip_stack_.emplace_back(value);
  } else {
    clip_stack_.back() = value;
  }
}

bool HWCanvasState::ClipStackEmpty() { return clip_stack_.empty(); }

bool HWCanvasState::NeedRevertClipStencil() {
  return !clip_stack_.empty() &&
         clip_stack_.back().stack_depth == matrix_state_.size();
}

HWCanvasState::ClipStackValue HWCanvasState::CurrentClipStackValue() {
  if (ClipStackEmpty()) {
    return ClipStackValue();
  } else {
    return clip_stack_.back();
  }
}

void HWCanvasState::ForEachClipStackValue(
    std::function<void(ClipStackValue const &, size_t)> const &func) {
  for (size_t i = 0; i < clip_stack_.size(); i++) {
    func(clip_stack_[i], i);
  }
}

Matrix HWCanvasState::CurrentMatrix() { return matrix_state_.back(); }

bool HWCanvasState::HasClip() { return !clip_stack_.empty(); }

bool HWCanvasState::MatrixDirty() { return matrix_dirty_; }

void HWCanvasState::ClearMatrixDirty() { matrix_dirty_ = false; }

void HWCanvasState::PushMatrixStack() {
  matrix_state_.emplace_back(CurrentMatrix());
}

void HWCanvasState::PopMatrixStack() { matrix_state_.pop_back(); }

void HWCanvasState::PopClipStack() {
  if (clip_stack_.empty()) {
    return;
  }

  if (clip_stack_.back().stack_depth <= matrix_state_.size()) {
    return;
  }

  clip_stack_.pop_back();
}

}  // namespace skity