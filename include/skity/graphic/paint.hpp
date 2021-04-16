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

  enum Style: uint8_t {
    kFill_Style,            /// set to fill geometry     
    kStroke_Style,          /// set to stroke geometry
    kStrokeAndFill_Style,   /// set to stroke and fill geometry
  };
};

}  // namespace skity

#endif  // SKITY_GRAPHIC_PAINT_HPP
