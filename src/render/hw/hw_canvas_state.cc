#include "src/render/hw/hw_canvas_state.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace skity {

HWCanvasState::HWCanvasState() {
  // init first stack matrix
  matrix_state_.emplace_back(glm::identity<Matrix>());
}

void HWCanvasState::Save() { PushMatrixStack(); }

void HWCanvasState::Restore() { PopMatrixStack(); }

void HWCanvasState::Translate(float dx, float dy) {
  Matrix current = CurrentMatrix();
  Matrix translate = glm::translate(glm::identity<Matrix>(), {dx, dy, 0.f});

  matrix_state_.back() = current * translate;
}

void HWCanvasState::Scale(float dx, float dy) {
  Matrix current = CurrentMatrix();
  Matrix scale = glm::scale(glm::identity<Matrix>(), {dx, dy, 1.f});

  matrix_state_.back() = current * scale;
}

void HWCanvasState::Rotate(float degree) {
  Matrix current = CurrentMatrix();
  Matrix rotate = glm::rotate(glm::identity<Matrix>(), glm::radians(degree),
                              {0.f, 0.f, 1.f});
  matrix_state_.back() = current * rotate;
}

void HWCanvasState::Rotate(float degree, float px, float py) {
  Matrix current = CurrentMatrix();
  Matrix rotate = glm::rotate(glm::identity<Matrix>(), glm::radians(degree),
                              {0.f, 0.f, 1.f});
  Matrix pre = glm::translate(glm::identity<Matrix>(), {-px, -py, 0.f});
  Matrix post = glm::translate(glm::identity<Matrix>(), {px, py, 0.f});

  matrix_state_.back() = current * post * rotate * pre;
}

void HWCanvasState::Concat(const Matrix &matrix) {
  Matrix current = CurrentMatrix();

  matrix_state_.back() = current * matrix;
}

Matrix HWCanvasState::CurrentMatrix() { return matrix_state_.back(); }

bool HWCanvasState::HasClip() { return false; }

void HWCanvasState::PushMatrixStack() {
  matrix_state_.emplace_back(CurrentMatrix());
}

void HWCanvasState::PopMatrixStack() { matrix_state_.pop_back(); }

}  // namespace skity