#ifndef SKITY_INCLUDE_SKITY_GEOMETRY_RRECT_HPP
#define SKITY_INCLUDE_SKITY_GEOMETRY_RRECT_HPP

#include <array>
#include <skity/geometry/point.hpp>
#include <skity/geometry/rect.hpp>

namespace skity {

class RRect {
 public:
  RRect() = default;
  RRect(RRect const& rrect) = default;
  RRect& operator=(RRect const& rrect) = default;

  /**
   * @enum RRect::Type
   *  Type describes possible specializations of SkRRect. Each Type is
   * 	exclusive; a RRect may only have one type.
   */
  enum Type {
    // zero width or height
    kEmpty,
    // non-zero width and height, and zeroed radii
    kRect,
    // non-zero width and height filled with radii
    kOval,
    // non-zero width and height with equal radii
    kSimple,
    // non-zero width and height with axis-aligned radii
    kNinePatch,
    // non-zero width and height with arbitrary radii
    kComplex,
    kLastType = kComplex,
  };

  /**
   * @enum RRect::Corner
   *	The radii are stored: top-left, top-right, bottom-right, bottom-left.
   */
  enum Corner {
    // index of top-left corner radii
    kUpperLeft,
    // index of top-right corner radii
    kUpperRight,
    // index of bottom-right corner radii
    kLowerRight,
    // index of bottom-left corner radii
    kLowerLeft,
  };

  Type getType() const;

  Type type() const { return this->getType(); }

  bool isEmpty() const { return Type::kEmpty == this->getType(); }
  bool isRect() const { return Type::kRect == this->getType(); }
  bool isOval() const { return Type::kOval == this->getType(); }
  bool isSimple() const { return Type::kSimple == this->getType(); }
  bool isNinePatch() const { return Type::kNinePatch == this->getType(); }
  bool isComplex() const { return Type::kComplex == this->getType(); }

  float width() const { return rect_.width(); }
  float height() const { return rect_.height(); }
  Vec2 getSimpleRadii() const { return radii_[0]; }

  void setEmpty() { *this = RRect(); }

  /**
   * @brief Set the Rect object
   *
   * @param rect bounds to set
   */
  void setRect(Rect const& rect);

  /**
   * Sets bounds to oval, x-axis radii to half oval.width(), and all y-axis
   * radii to half oval.height(). If oval bounds is empty, sets to kEmpty.
   * Otherwise, sets to kOval.
   *
   * @param oval  bounds of oval
   */
  void setOval(Rect const& oval);

  /**
   * @brief Sets to rounded rectangle with the same radii for all four corners.
   *
   * @param rect  bounds of rounded rectangle
   * @param xRad  x-axis radius of corners
   * @param yRad  y-axis radius of corners
   */
  void setRectXY(Rect const& rect, float xRad, float yRad);

  Rect const& rect() const { return rect_; }

  /**
   * Check if bounds and radii match type
   *
   * @return true
   * @return false
   */
  bool isValid() const;

  Vec2 radii(Corner corner) const { return radii_[corner]; }

  /**
   * Translates RRect by (dx, dy)
   *
   * @param dx  offset added to rect().left() and rect().right()
   * @param dy  offset added to rect().top() and rect().bottom()
   */
  void offset(float dx, float dy) { rect_.offset(dx, dy); }

  /**
   * Returns bounds. Bounds may have zero width or zero height
   *
   * @return bounding box
   */
  Rect const& getBounds() const { return rect_; }

  static RRect MakeEmpty();
  static RRect MakeRect(Rect const& r);
  static RRect MakeRectXY(Rect const& rect, float xRad, float yRad);
  static RRect MakeOval(Rect const& oval);

 private:
  RRect(Rect const& rect, std::array<Vec2, 4> const& radii, Type type)
      : rect_(rect), radii_(radii), type_(type) {}

  static bool AreRectAndRadiiValid(Rect const& rect,
                                   std::array<Vec2, 4> const& radii);

  bool initializeRect(Rect const& rect);
  void computeType();
  bool checkCornerContainment(float x, float y) const;
  // return true if the radii had to be scaled to fit rect
  bool scaleRadii();

 private:
  Rect rect_ = Rect::MakeEmpty();
  std::array<Vec2, 4> radii_ = {};
  Type type_ = Type::kEmpty;
};

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GEOMETRY_RRECT_HPP