#include "skity/render/canvas.hpp"

#include <skity/text/utf.hpp>

namespace skity {

Canvas::Canvas() { default_typeface_ = Typeface::MakeDefault(); }

Canvas::~Canvas() = default;

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

void Canvas::restoreToCount(int saveCount) {
  if (saveCount >= save_count_) {
    return;
  }

  for (int32_t i = save_count_; i >= saveCount; i--) {
    this->restore();
  }

  assert(save_count_ == saveCount);
}

void Canvas::translate(float dx, float dy) { onTranslate(dx, dy); }

void Canvas::scale(float sx, float sy) { onScale(sx, sy); }

void Canvas::rotate(float degrees) { onRotate(degrees); }

void Canvas::rotate(float degrees, float px, float py) {
  onRotate(degrees, px, py);
}

void Canvas::skew(float sx, float sy) {}

void Canvas::concat(const Matrix &matrix) { onConcat(matrix); }

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

void Canvas::drawCircle(float cx, float cy, float radius, Paint const &paint) {
  if (radius < 0) {
    radius = 0;
  }

  Rect r;
  r.setLTRB(cx - radius, cy - radius, cx + radius, cy + radius);
  this->drawOval(r, paint);
}

void Canvas::drawOval(Rect const &oval, Paint const &paint) {
  RRect rrect;
  rrect.setOval(oval);
  this->drawRRect(rrect, paint);
}

void Canvas::drawRect(Rect const &rect, Paint const &paint) {
  Path path;
  path.addRect(rect);

  this->drawPath(path, paint);
}

void Canvas::drawRRect(RRect const &rrect, Paint const &paint) {
  Path path;
  path.addRRect(rrect);

  this->drawPath(path, paint);
}

void Canvas::drawRoundRect(Rect const &rect, float rx, float ry,
                           Paint const &paint) {
  if (rx > 0 && ry > 0) {
    RRect rrect;
    rrect.setRectXY(rect, rx, ry);
    this->drawRRect(rrect, paint);
  } else {
    this->drawRect(rect, paint);
  }
}

void Canvas::drawPath(const Path &path, const Paint &paint) {
  this->onDrawPath(path, paint);
}

void Canvas::flush() { this->onFlush(); }

void Canvas::drawSimpleText(const char *text, float x, float y,
                            Paint const &paint) {
#if 0
  if (!default_typeface_) {
    return;
  }
  this->save();

  this->translate(x, y);

  std::vector<GlyphInfo> glyphs;
  std::vector<GlyphID> glyph_id;
  default_typeface_->textToGlyphId(text, glyph_id);
  default_typeface_->getGlyphInfo(glyph_id, paint.getTextSize(), glyphs);

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

void Canvas::drawSimpleText2(const char *text, float x, float y,
                             const Paint &paint) {
  if (!default_typeface_) {
    return;
  }

  std::vector<GlyphInfo> glyphs;
  std::vector<GlyphID> glyph_id;
  if (!UTF::UTF8ToCodePoint(text, std::strlen(text), glyph_id)) {
    return;
  }

  this->save();

  this->translate(x, y);

  default_typeface_->getGlyphInfo(glyph_id, paint.getTextSize(), glyphs);

  this->onDrawGlyphs(glyphs, default_typeface_.get(), paint);

  this->restore();
}

float Canvas::simpleTextBounds(const char *text, const Paint &paint) {
  std::vector<GlyphInfo> glyphs;
  std::vector<GlyphID> glyph_id;
  if (!UTF::UTF8ToCodePoint(text, std::strlen(text), glyph_id)) {
    return 0.f;
  }
  default_typeface_->getGlyphInfo(glyph_id, paint.getTextSize(), glyphs);
  float total_width = 0.f;
  for (auto &glyph : glyphs) {
    total_width += glyph.advance_x;
  }
  return total_width;
}

void Canvas::updateViewport(uint32_t width, uint32_t height) {
  this->onUpdateViewport(width, height);
}

uint32_t Canvas::width() const { return this->onGetWidth(); }

uint32_t Canvas::height() const { return this->onGetHeight(); }

void Canvas::internalSave() { this->onSave(); }

void Canvas::internalRestore() { this->onRestore(); }

}  // namespace skity
