#ifndef SKITY_EFFECT_SHADER_HPP
#define SKITY_EFFECT_SHADER_HPP

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <skity/geometry/point.hpp>
#include <vector>

namespace skity {

class Pixmap;

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

  void SetLocalMatrix(Matrix const& matrix) { local_matrix_ = matrix; }
  Matrix GetLocalMatrix() const { return local_matrix_; }

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
    Matrix local_matrix;
    /**
     * By default gradients will interpolate their colors in unpremul space
     *  and then premultiply each of the results. By setting this flag to 1, the
     *  gradients will premultiply their colors first, and then interpolate
     *  between them.
     *
     */
    int32_t gradientFlags;
  };

  virtual GradientType asGradient(GradientInfo* info) const;

  virtual std::shared_ptr<Pixmap> asImage() const;

  /**
   * Returns a shader that generates a linear gradient between the two specified
   * points.
   *
   * @param pts     The start and end points for the gradient.
   * @param colors  The array[count] of colors, to be distributed bteween the
   *                two points
   * @param pos     May be null or array[count] of floats for the relative
   *                position of each corresponding color in the colors array.
   * @param count   Must be >= 2. The number of colors (and pos if not NULL)
   *                entries.
   * @param flag    if set to 1, the gradients will premultiply their colors
   *                first, and then interpolate between them
   * @return        Then gradient shader instance
   */
  static std::shared_ptr<Shader> MakeLinear(const Point pts[2],
                                            const Vec4 colors[],
                                            const float pos[], int count,
                                            int flag = 0);

  /**
   * Returns a shader that generates a radial gradient given the center and
   * radius.
   *
   * @param center  The center of the circle for this gradient
   * @param radius  Must be positive. The radius of the circle for this gradient
   * @param colors  The array[count] of colors, to be distributed between the
   *                center and edge of the circle
   * @param pos     The array[count] of floats, or NULL. For the relative
   *                position of each corresponding color in the colors array.
   * @param count   Must be >= 2. The number of colors
   * @param flag    if set to 1, the gradients will premultiply their colors
   *                first, and then interpolate between them
   * @return        The gradient shader instance
   */
  static std::shared_ptr<Shader> MakeRadial(Point const& center, float radius,
                                            const Vec4 colors[],
                                            const float pos[], int count,
                                            int flag = 0);

  /**
   * Return a shader that fill color with pixel content
   *
   * @param pixmap
   * @return std::shared_ptr<Shader>
   */
  static std::shared_ptr<Shader> MakeShader(std::shared_ptr<Pixmap> pixmap);

 private:
  Matrix local_matrix_ = glm::identity<Matrix>();
};

}  // namespace skity

#endif  // SKITY_EFFECT_SHADER_HPP
