#ifndef SKITY_INCLUDE_SKITY_GEOMETRY_RECT_HPP
#define SKITY_INCLUDE_SKITY_GEOMETRY_RECT_HPP

#include <skity/geometry/point.hpp>
#include <skity/macros.hpp>

namespace skity {

class SK_API Rect {
 public:
  Rect() : Rect{0, 0, 0, 0} {}
  Rect(float left, float top, float right, float bottom)
      : left_{left}, top_{top}, right_{right}, bottom_{bottom} {}
  Rect(const Rect&) = default;
  Rect(Rect&&) = default;
  Rect& operator=(const Rect&) = default;

  bool operator==(const Rect& other) const {
    return this->left_ == other.left_ && this->top_ == other.top_ &&
           this->right_ == other.right_ && this->bottom_ == other.bottom_;
  }
  /**
   * Returns left edge of Rect, if sorted.
   * @return left
   */
  float x() const { return left_; }
  float left() const { return left_; }
  /**
   * Returns top edge of Rect.
   * @return top
   */
  float y() const { return top_; }
  float top() const { return top_; }
  /**
   * Returns right edge of Rect.
   * @return right
   */
  float right() const { return right_; }
  /**
   * Returns bottom edge of rect.
   * @return bottom
   */
  float bottom() const { return bottom_; }
  /**
   * Returns span on the x-axis. This dose not check if Rect is sorted.
   * Result may be negative or infinity.
   * @return
   */
  float width() const { return right_ - left_; }
  /**
   * Returns span on the y-axis.
   * @return
   */
  float height() const { return bottom_ - top_; }
  float centerX() const;
  float centerY() const;

  /**
   * Returns true if left is equal to or greater than right, or if top is equal
   * to or greater than bottom. Call sort() to reverse rectangles with negative
   * width() or height().
   * @return
   */
  bool isEmpty() const { return !(left_ < right_ && top_ < bottom_); }
  void setEmpty() { *this = MakeEmpty(); }
  /**
   * Returns for points in quad that enclose Rect ordered as: top-left,
   * top-right, bottom-right, bottom-left
   * @param quad
   */
  void toQuad(Point quad[4]) const;
  /**
   * Set Rect to (left, top, right, bottom)
   * @note left,right and top,bottom need to be sorted.
   * @param left    sorted in left
   * @param top     sorted in top
   * @param right   sorted in right
   * @param bottom  sorted in bottom
   */
  void setLTRB(float left, float top, float right, float bottom) {
    left_ = left;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
  }
  /**
   * Sets to bounds of Point array with count entries.
   * If count is zero or smaller, or if Point array contains an infinity or NaN,
   * sets to (0, 0, 0, 0).
   *
   * Result is either empty or sorted.
   *
   * @param pts     Point array
   * @param count   entries in array
   */
  void setBounds(const Point pts[], int count) {
    this->setBoundsCheck(pts, count);
  }
  bool setBoundsCheck(const Point pts[], int count);

  void set(const Point& p0, const Point& p1) {
    left_ = glm::min(p0.x, p1.x);
    right_ = glm::max(p0.x, p1.x);
    top_ = glm::min(p0.y, p1.y);
    bottom_ = glm::max(p0.y, p1.y);
  }

  /**
   * Sets Rect to (x, y, x + width, y + height)
   * Dose not validate input; with or height may be nagtive.
   *
   * @param x       stored in left
   * @param y       stored in top
   * @param width   added to x and stored in right
   * @param height  added to y and stored in bottom
   */
  void setXYWH(float x, float y, float width, float height) {
    left_ = x;
    top_ = y;
    right_ = x + width;
    bottom_ = y + height;
  }
  void setWH(float width, float height) { this->setXYWH(0, 0, width, height); }

  void offset(float dx, float dy) {
    left_ += dx;
    top_ += dy;
    right_ += dx;
    bottom_ += dy;
  }

  bool isSorted() const { return left_ <= right_ && top_ <= bottom_; }

  bool isFinite() const;

  void sort() {
    if (left_ > right_) {
      std::swap(left_, right_);
    }
    if (top_ > bottom_) {
      std::swap(top_, bottom_);
    }
  }

  Rect makeSorted() const;

  /**
   * Sets Rect to the union of itself and r.
   * @param r expansion Rect
   */
  void join(Rect const& r);

  static Rect MakeEmpty() { return Rect{0, 0, 0, 0}; }

  static Rect MakeWH(float width, float height) {
    return Rect{0, 0, width, height};
  }

  static Rect MakeLTRB(float l, float t, float r, float b) {
    return Rect{l, t, r, b};
  }

  static Rect MakeXYWH(float x, float y, float w, float h) {
    return Rect{x, y, x + w, y + h};
  }

  static float HalfWidth(Rect const& rect);

  static float HalfHeight(Rect const& rect);

 private:
  float left_;
  float top_;
  float right_;
  float bottom_;
};

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GEOMETRY_RECT_HPP