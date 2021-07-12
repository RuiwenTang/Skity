#include "skity/render/canvas.hpp"

namespace skity {

Canvas::Canvas() {
#ifdef ENABLE_TEXT_RENDER
  ft_typeface_ = ft_library_.LoadTypeface(BUILD_IN_FONT_FILE);
#endif
}

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

void Canvas::translate(float dx, float dy) { onTranslate(dx, dy); }

void Canvas::scale(float sx, float sy) { onScale(sx, sy); }

void Canvas::rotate(float degrees) { onRotate(degrees); }

void Canvas::rotate(float degrees, float px, float py) {
  onRotate(degrees, px, py);
}

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

void Canvas::flush() { this->onFlush(); }

void Canvas::drawSimpleText(const char *text, float x, float y,
                            Paint const &paint) {
#ifdef ENABLE_TEXT_RENDER
  this->save();

  this->translate(x, y);

  auto glyphs = ft_typeface_->LoadGlyph(text);
  uint32_t index = 0;
  for (; index < glyphs.size(); index++) {
    auto const &info = glyphs[index];
    if (index > 0) {
      this->translate(glyphs[index - 1].advance_x, 0);
    }

    this->drawPath(info.path, paint);
  }

  this->restore();
#endif
}

void Canvas::internalSave() { this->onSave(); }

void Canvas::internalRestore() { this->onRestore(); }

}  // namespace skity