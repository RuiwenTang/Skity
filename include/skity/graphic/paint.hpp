#ifndef SKITY_GRAPHIC_PAINT_HPP
#define SKITY_GRAPHIC_PAINT_HPP

#include <cstdint>

namespace skity {

/**
 * @class Paint
 * Controls options applied when drawing.
 */
class Paint {
 public:
  Paint();
  ~Paint();

  Paint& operator=(const Paint& paint);

  void reset();

  enum Style : std::uint8_t {
    kFill_Style,           /// set to fill geometry
    kStroke_Style,         /// set to stroke geometry
    kStrokeAndFill_Style,  /// set to stroke and fill geometry
  };
  // may be used to verify that Paint::Style is a legal value
  static constexpr int32_t StyleCount = kStrokeAndFill_Style + 1;

  Style getStyle() const;

  void setStyle(Style style);

  /**
   * Set the thickness of the pen used by the paint to outline the shape.
   * @TODO may be support stroke-width of zero as hairline
   * @param width pen thickness
   */
  void setStrokeWidth(float width);

  float getStrokeWidth() const;

  float getStrokeMiter() const;

  /**
   * Set the limit at which a sharp corner is drawn beveled.
   *
   * @param miter
   */
  void setStrokeMiter(float miter);

  /**
   * @enum Paint::Cap
   *
   * Cap draws at the beginning and end of an open path contour.
   */
  enum Cap : std::uint8_t {
    kButt_Cap,                 /// no stroke extension
    kRound_Cap,                /// add circle
    kSquare_Cap,               /// add square
    kLast_Cap = kSquare_Cap,   /// largest Cap value
    kDefault_Cap = kButt_Cap,  /// equivalent to kButt_Cap
  };

  static constexpr std::int32_t kCapCount = kLast_Cap + 1;

  Cap getStrokeCap() const;

  void setStrokeCap(Cap cap);

  /**
   * @enum Paint::Join
   *
   * Join specifies how corners are drawn when a shape is stroked.
   */
  enum Join : std::uint8_t {
    kMiter_Join,    /// extends to miter limit
    kRound_Join,    /// add circle
    kBevel_Join,    /// connects outside edges
    kLast_Join,     /// equivalent to the largest value for Join
    kDefault_Join,  /// equivalent to kMiter_Join
  };

  static constexpr std::int32_t kJoinCount = kLast_Join + 1;

  Join getStrokeJoin() const;

  void setStrokeJoin(Join join);

 private:
  Cap cap_ = kDefault_Cap;
  Join join_ = kDefault_Join;
  Style style_ = kFill_Style;
  float stroke_width_ = 1.0f;
  float miter_limit_ = 0.f;
};

}  // namespace skity

#endif  // SKITY_GRAPHIC_PAINT_HPP
