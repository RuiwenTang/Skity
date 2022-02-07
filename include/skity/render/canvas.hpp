
#ifndef SKITY_RENDER_CANVAS_HPP
#define SKITY_RENDER_CANVAS_HPP

#include <memory>
#include <skity/geometry/rect.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/macros.hpp>
#include <skity/text/typeface.hpp>

namespace skity {

class TextBlob;
class GPUContext;

/**
 * @class Canvas
 * Provide an interface for drawing.
 */
class SK_API Canvas {
 public:
  Canvas();
  virtual ~Canvas();

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

  /**
   * Replaces Matrix with matrix premultiplied with existing Matrix.
   * @param matrix matrix to premultiply with existing Matrix
   */
  void concat(const Matrix& matrix);

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

  /**
   * Draws circle at center with radius using clip, Matrix, and Paint paint.
   *
   * @param cx      circle center on the x-axis
   * @param cy      circle center on the y-axis
   * @param radius  half the diameter of circle
   * @param paint   Paint stroke or fill, blend, color, and so on, used to draw
   */
  void drawCircle(float cx, float cy, float radius, Paint const& paint);

  /**
   * Draws oval oval using clip, Matrix, and Paint.
   *
   * @param oval    Rect bounds of oval
   * @param paint   Paint stroke or fill, blend, color, and so on, used to draw
   */
  void drawOval(Rect const& oval, Paint const& paint);

  /**
   * Draws Rect rect using clip, Matrix, and Paint paint.
   *
   * @param rect  rectangle to draw
   * @param paint stroke or fill, blend, color, and so on, used to draw
   */
  void drawRect(Rect const& rect, Paint const& paint);

  /**
   * Draws RRect rrect using clip, Matrix, and Paint paint.
   *
   * @param rrect RRect with up to eight corner radii to draw
   * @param paint Paint stroke or fill, blend, color, and so on, used to draw
   */
  void drawRRect(RRect const& rrect, Paint const& paint);

  /**
   * Draws RRect bounded by Rect rect, with corner radii (rx, ry) using clip,
   * Matrix, and Paint paint.
   *
   * @param rect    Rect bounds of RRect to draw
   * @param rx      axis length of x-axis of oval describing rounded corners
   * @param ry      axis length on y-axis of oval describing rounded corners
   * @param paint   stroke, blend, color, and so on, used to draw
   */
  void drawRoundRect(Rect const& rect, float rx, float ry, Paint const& paint);

  void drawPath(Path const& path, Paint const& paint);

  /**
   * @brief Flush the internal draw commands.
   * @note this function must be called if Canvas is create with GPU backend.
   *         After this funcion called on OpenGL backends:
   *                the stencil buffer maybe dirty and need clean
   *                the stencil mask、stencil func and stencil op is changed
   *                the color mask need reset.
   *         After this function called on Vulkan backends:
   *                the current VkCommandBuffer is filled with draw commands
   *                the binded VkPipeline is changed
   *
   */
  void flush();

  /**
   * @brief Set the Default Typeface object
   *        If no Typeface is profide by Paint, then the default Typeface is
   *        used for drawing text.
   *
   * @param typeface the default Typeface for draw text
   */
  void setDefaultTypeface(std::shared_ptr<Typeface> typeface);

  std::shared_ptr<Typeface> const& getDefaultTypeface() const {
    return default_typeface_;
  }

  /**
   * @deprecated  use drawSimpleText2 if need.
   */
  void drawSimpleText(const char* text, float x, float y, Paint const& paint);

  /**
   * @deprecated  use drawTextBlob instead
   * @brief       this function is fallback to use drawTextBlob internal.
   *
   */
  void drawSimpleText2(const char* text, float x, float y, Paint const& paint);

  Vec2 simpleTextBounds(const char* text, Paint const& paint);

  void drawTextBlob(const TextBlob* blob, float x, float y, Paint const& paint);

  inline void drawDebugLine(bool debug) { draw_debug_line_ = debug; }

  void updateViewport(uint32_t width, uint32_t height);
  uint32_t width() const;
  uint32_t height() const;

  /**
   * Create a Canvas instance with GPU backend.
   * For all GPU backends, Skity need to enable stencil test and color blend.
   *
   * @param width   the total width of the render target
   * @param height  the total height of the render target
   * @param density pixel density for the current physical device
   * @param ctx     GPUContext pointer, if build for Vulkan, this need to be a
   *                GPUVkContext pointer
   * @return Canvas instance
   */
  static std::unique_ptr<Canvas> MakeHardwareAccelationCanvas(uint32_t width,
                                                              uint32_t height,
                                                              float density,
                                                              GPUContext* ctx);

 protected:
  // default implement dispatch this to onClipPath
  virtual void onClipRect(Rect const& rect, ClipOp op);
  virtual void onClipPath(Path const& path, ClipOp op) = 0;

  // default implement dispatch this to onDrawPath
  virtual void onDrawLine(float x0, float y0, float x1, float y1,
                          Paint const& paint);
  // default implement dispatch this to onDrawPath
  virtual void onDrawCircle(float cx, float cy, float radius,
                            Paint const& paint);
  // default implement dispatch this to onDrawPath
  virtual void onDrawOval(Rect const& oval, Paint const& paint);
  // default implement dispatch this to onDrawPath
  virtual void onDrawRect(Rect const& rect, Paint const& paint);
  // default implement dispatch this to onDrawPath
  virtual void onDrawRRect(RRect const& rrect, Paint const& paint);
  // default implement dispatch this to onDrawPath
  virtual void onDrawRoundRect(Rect const& rect, float rx, float ry,
                               Paint const& paint);

  virtual void onDrawPath(Path const& path, Paint const& paint) = 0;

  virtual void onDrawBlob(const TextBlob* blob, float x, float y,
                          Paint const& paint) = 0;

  virtual void onSave() = 0;
  virtual void onRestore() = 0;
  virtual void onRestoreToCount(int saveCount) = 0;
  virtual void onTranslate(float dx, float dy) = 0;
  virtual void onScale(float sx, float sy) = 0;
  virtual void onRotate(float degree) = 0;
  virtual void onRotate(float degree, float px, float py) = 0;
  virtual void onConcat(Matrix const& matrix) = 0;
  virtual void onFlush() = 0;
  virtual uint32_t onGetWidth() const = 0;
  virtual uint32_t onGetHeight() const = 0;

  virtual bool needGlyphPath(Paint const& paint);

  virtual void onUpdateViewport(uint32_t width, uint32_t height) = 0;
  inline bool isDrawDebugLine() const { return draw_debug_line_; }

 private:
  void internalSave();
  void internalRestore();

 private:
  uint32_t save_count_ = 0;
  bool draw_debug_line_ = false;
  std::shared_ptr<Typeface> default_typeface_ = {};
};

}  // namespace skity

#endif  // SKITY_RENDER_CANVAS_HPP
