#include "skity/render/canvas.hpp"

namespace skity {

int Canvas::save() {
  save_count_ += 1;
  this->internalSave();
  return this->getSaveCount() - 1;
}

void Canvas::restore() {
  if (save_count_ > 0) {
    save_count_ -= 1;
    this->internalRestore();
  }
}

int Canvas::getSaveCount() const { return save_count_; }

void Canvas::restoreToCount(int saveCount) {}

void Canvas::translate(float dx, float dy) {}

void Canvas::scale(float sx, float sy) {}

void Canvas::rotate(float degrees) {}

void Canvas::rotate(float degrees, float px, float py) {}

void Canvas::skew(float sx, float sy) {}

void Canvas::clipRect(const Rect &rect, ClipOp op) {
  Path path;
  path.addRect(rect);
  this->onClipPath(path, op);
}

void Canvas::clipPath(const Path &path, ClipOp op) {
  this->onClipPath(path, op);
}

void Canvas::drawLine(float x0, float y0, float x1, float y1,
                      const Paint &paint) {
  Path path;
  path.moveTo(x0, y0);
  path.lineTo(x1, y1);

  drawPath(path, paint);
}

void Canvas::drawPath(const Path &path, const Paint &paint) {
  this->onDrawPath(path, paint);
}

void Canvas::internalSave() { this->onSave(); }

void Canvas::internalRestore() { this->onRestore(); }

}  // namespace skity