
#ifndef SKITY_RENDER_CANVAS_HPP
#define SKITY_RENDER_CANVAS_HPP

#include <memory>
#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity_config.hpp>

#ifdef ENABLE_TEXT_RENDER
#include "src/render/text/ft_library_wrap.hpp"
#endif

namespace skity {

/**
 * @class Canvas
 * Provide an interface for drawing.
 */
class Canvas {
 public:
  Canvas();
  virtual ~Canvas() = default;

  /**
   * Save Matrix and clip.
   *
   * @return  depth of saved stack
   */
  int save();

  /**
   * Removes changes to Matrix and clip sice canvas was last saved.
   */
  void restore();

  /**
   * @return depth of save state stack
   */
  int getSaveCount() const;

  /**
   * Restores state to Matrix and clip values when save() returned saveCount.
   *
   * Does nothing if saveCount is greater than state stack count.
   * @param saveCount
   */
  void restoreToCount(int saveCount);

  /**
   * Translates Matrix by dx along the x-axis and dy along the y-axis.
   *
   * @param dx distance to translate on x-axis
   * @param dy distance to translate on y-axis
   */
  void translate(float dx, float dy);

  /**
   * Scale Matrix by sx on x-asix and sy on y-axis
   *
   * @param sx amount to scale on x-axis
   * @param sy amount to scale on y-axis
   */
  void scale(float sx, float sy);

  /**
   * Rotate Matrix by degrees. Positive degrees rotates clockwise.
   *
   * @param degrees amount to rotate, in degrees
   */
  void rotate(float degrees);

  /**
   * Rotates Matrix by degrees about a point at(px, py).
   *
   * @param degrees amount to rotate, in degrees
   * @param px      x-axis value of the point to rotate about
   * @param py      y-axis value of the point to rotate about
   */
  void rotate(float degrees, float px, float py);

  /**
   * Skews Matrix by sx on x-axis and sy on the y-axis.
   *
   * @param sx  amount to skew on x-axis
   * @param sy  amount to skew on y-axis
   */
  void skew(float sx, float sy);

  enum class ClipOp {
    kDifference,
    kIntersect,
  };

  /**
   * Replaces clip with the intersection or difference of clip and rect
   *
   * @param rect  Rect to combine with clip
   * @param op    ClipOp to apply to clip
   */
  void clipRect(Rect const& rect, ClipOp op = ClipOp::kIntersect);

  void clipPath(Path const& path, ClipOp op = ClipOp::kIntersect);

  /**
   * Draws line segment from (x0, y0) to (x1, y1) using clip, Matrix, and paint.
   *
   * In paint: stroke width describes the line thicknes;
   * Paint::Cap draws the end rounded or square;
   * Paint::Style is ignored, as if were set to Paint::Style::kStroke_Style.
   *
   * @param x0    start of line segment on x-axis
   * @param y0    start of line segment on y-axis
   * @param x1    end of line segment on x-axis
   * @param y1    end of line segment on y-axis
   * @param paint stroke, blend, color, and so on, used to draw
   */
  void drawLine(float x0, float y0, float x1, float y1, Paint const& paint);

  void drawPath(Path const& path, Paint const& paint);

  void flush();

  // just draw text not a usable interface
  void drawSimpleText(const char* text, float x, float y, Paint const& paint);

  inline void drawDebugLine(bool debug) { draw_debug_line_ = debug; }

  static std::unique_ptr<Canvas> MakeGLCanvas(uint32_t x, uint8_t y,
                                              uint32_t width, uint32_t height);

 protected:
  virtual void onClipPath(Path const& path, ClipOp op) = 0;
  virtual void onDrawPath(Path const& path, Paint const& paint) = 0;
  virtual void onSave() = 0;
  virtual void onRestore() = 0;
  virtual void onTranslate(float dx, float dy) = 0;
  virtual void onScale(float sx, float sy) = 0;
  virtual void onRotate(float degree) = 0;
  virtual void onRotate(float degree, float px, float py) = 0;
  virtual void onFlush() = 0;

  inline bool isDrawDebugLine() const { return draw_debug_line_; }

 private:
  void internalSave();
  void internalRestore();

 private:
  uint32_t save_count_ = 0;
  bool draw_debug_line_ = false;
#ifdef ENABLE_TEXT_RENDER
  FTLibrary ft_library_;
  std::unique_ptr<FTTypeFace> ft_typeface_;
#endif
};

}  // namespace skity

#endif  // SKITY_RENDER_CANVAS_HPP
