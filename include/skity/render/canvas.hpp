
#ifndef SKITY_RENDER_CANVAS_HPP
#define SKITY_RENDER_CANVAS_HPP

#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

/**
 * @class Canvas
 * Privide an interface for drawing.
 */
class Canvas {
 public:
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
};

}  // namespace skity

#endif  // SKITY_RENDER_CANVAS_HPP

