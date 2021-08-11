#ifndef SKITY_EFFECT_SHADER_HPP
#define SKITY_EFFECT_SHADER_HPP

#include <array>
#include <skity/geometry/point.hpp>
#include <vector>

namespace skity {

/**
 * Shaders specify the source color(s) for what is being drawn.
 * if a paint has no shader, then the paint's color is used. If the paint has a
 * shader, then the shader's color(s) are use instead.
 */
class Shader {
 public:
  Shader() = default;
  virtual ~Shader() = default;

  virtual bool isOpaque() const { return false; }

  enum GradientType {
    kNone,
    kColor,
    kLinear,
    kRadial,
    kSweep,
    kConical,
  };

  struct GradientInfo {
    int32_t color_count;
    std::vector<Vec4> colors;
    std::vector<float> color_offsets;
    std::array<Point, 2> point;
    std::array<float, 2> radius;
  };

  virtual GradientType asGradient(GradientInfo* info) const;
};

}  // namespace skity

#endif  // SKITY_EFFECT_SHADER_HPP
