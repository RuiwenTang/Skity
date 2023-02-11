#include "skity/render/canvas.hpp"

#include <cstring>
#include <skity/text/text_blob.hpp>
#include <skity/text/utf.hpp>

namespace skity {

Canvas::Canvas() = default;

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

  save_count_ = saveCount;
  this->onRestoreToCount(saveCount);
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
  this->onClipRect(rect, op);
}

void Canvas::onClipRect(const Rect &rect, ClipOp op) {
  Path path;
  path.addRect(rect);
  path.setConvexityType(Path::ConvexityType::kConvex);

  this->onClipPath(path, op);
}

void Canvas::clipPath(const Path &path, ClipOp op) {
  this->onClipPath(path, op);
}

void Canvas::drawLine(float x0, float y0, float x1, float y1,
                      const Paint &paint) {
  this->onDrawLine(x0, y0, x1, y1, paint);
}

void Canvas::drawCircle(float cx, float cy, float radius, Paint const &paint) {
  this->onDrawCircle(cx, cy, radius, paint);
}

void Canvas::drawOval(Rect const &oval, Paint const &paint) {
  this->onDrawOval(oval, paint);
}

void Canvas::drawRect(Rect const &rect, Paint const &paint) {
  this->onDrawRect(rect, paint);
}

void Canvas::drawRRect(RRect const &rrect, Paint const &paint) {
  this->onDrawRRect(rrect, paint);
}

void Canvas::drawRoundRect(Rect const &rect, float rx, float ry,
                           Paint const &paint) {
  this->onDrawRoundRect(rect, rx, ry, paint);
}

void Canvas::drawPath(const Path &path, const Paint &paint) {
  this->onDrawPath(path, paint);
}

void Canvas::flush() { this->onFlush(); }

void Canvas::setDefaultTypeface(std::shared_ptr<Typeface> typeface) {
  default_typeface_ = std::move(typeface);
}

void Canvas::drawSimpleText(const char *text, float x, float y,
                            Paint const &paint) {
  this->drawSimpleText2(text, x, y, paint);
}

void Canvas::drawSimpleText2(const char *text, float x, float y,
                             const Paint &paint) {
  if (!default_typeface_ && !paint.getTypeface()) {
    return;
  }

  auto typeface = default_typeface_;

  if (paint.getTypeface()) {
    typeface = paint.getTypeface();
  }

  Paint work_paint{paint};
  work_paint.setTypeface(typeface);

  skity::TextBlobBuilder builder;

  auto blob = builder.buildTextBlob(text, work_paint);

  this->drawTextBlob(blob.get(), x, y, work_paint);
}

Vec2 Canvas::simpleTextBounds(const char *text, const Paint &paint) {
  std::vector<GlyphInfo> glyphs;
  std::vector<GlyphID> glyph_id;
  if (!UTF::UTF8ToCodePoint(text, std::strlen(text), glyph_id)) {
    return {0.f, 0.f};
  }

  auto typeface = default_typeface_.get();

  if (paint.getTypeface()) {
    typeface = paint.getTypeface().get();
  }

  typeface->getGlyphInfo(glyph_id, paint.getTextSize(), glyphs);
  float total_width = 0.f;
  float max_height = 0.f;
  for (auto &glyph : glyphs) {
    total_width += glyph.advance_x;
    max_height = std::max(max_height, glyph.ascent - glyph.descent);
  }
  return {total_width, max_height};
}

void Canvas::drawTextBlob(const TextBlob *blob, float x, float y,
                          const Paint &paint) {
  this->onDrawBlob(blob, x, y, paint);
}

void Canvas::updateViewport(uint32_t width, uint32_t height) {
  this->onUpdateViewport(width, height);
}

uint32_t Canvas::width() const { return this->onGetWidth(); }

uint32_t Canvas::height() const { return this->onGetHeight(); }

void Canvas::internalSave() { this->onSave(); }

void Canvas::internalRestore() { this->onRestore(); }

void Canvas::onDrawLine(float x0, float y0, float x1, float y1,
                        Paint const &paint) {
  Path path;
  path.moveTo(x0, y0);
  path.lineTo(x1, y1);

  this->onDrawPath(path, paint);
}

void Canvas::onDrawCircle(float cx, float cy, float radius,
                          Paint const &paint) {
  if (radius < 0) {
    radius = 0;
  }

  Rect r;
  r.setLTRB(cx - radius, cy - radius, cx + radius, cy + radius);
  this->drawOval(r, paint);
}

void Canvas::onDrawOval(Rect const &oval, Paint const &paint) {
  RRect rrect;
  rrect.setOval(oval);
  this->drawRRect(rrect, paint);
}

void Canvas::onDrawRRect(RRect const &rrect, Paint const &paint) {
  Path path;
  path.addRRect(rrect);
  path.setConvexityType(Path::ConvexityType::kConvex);

  this->drawPath(path, paint);
}

void Canvas::onDrawRect(Rect const &rect, Paint const &paint) {
  Path path;
  path.addRect(rect);

  this->drawPath(path, paint);
}

void Canvas::onDrawRoundRect(Rect const &rect, float rx, float ry,
                             Paint const &paint) {
  if (rx > 0 && ry > 0) {
    RRect rrect;
    rrect.setRectXY(rect, rx, ry);
    this->drawRRect(rrect, paint);
  } else {
    this->drawRect(rect, paint);
  }
}

bool Canvas::needGlyphPath(Paint const &paint) {
  return paint.getStyle() != Paint::kFill_Style;
}

}  // namespace skity
