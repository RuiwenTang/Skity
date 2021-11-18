#ifndef SKITY_SRC_RENDER_HW_HW_SHADER_HPP
#define SKITY_SRC_RENDER_HW_HW_SHADER_HPP

#include <glm/glm.hpp>
#include <skity/graphic/color.hpp>
#include <vector>

namespace skity {

class HWShader {
 public:
  virtual ~HWShader() = default;

  /**
   * @brief Upload projection matrix to GPU shader
   *
   * @param mvp
   */
  virtual void SetViewProjectionMatrix(glm::mat4 const& mvp) = 0;

  /**
   * @brief Upload transform matrix to GPU shader
   *
   * @param matrix
   */
  virtual void SetModelMatrix(glm::mat4 const& matrix) = 0;

  /**
   * @brief ivec4 info property,
   *        [type, gradient_color_count, gradient_color_stop_count, text]
   *
   * @param info
   */
  virtual void SetInfoI(glm::ivec4 const& info) = 0;

  /**
   * @brief vec4 info1 property
   *        [stroke_width, TBD, TBD, TBD]
   *
   * @param info
   */
  virtual void SetInfoF1(glm::vec4 const& info) = 0;

  /**
   * @brief vec4 info2 property
   *        [p1.x, p1.y, p2.x, p2.y]
   *
   * @param info
   */
  virtual void SetInfo2(glm::vec4 const& info) = 0;

  /**
   * @brief Upload gradient colors to GPU shader
   *
   * @param colors
   */
  virtual void SetColors(std::vector<Color4f> const& colors) = 0;

  /**
   * @brief Upload gradient positions to GPU shader
   *
   * @param pos
   */
  virtual void SetPositions(std::vector<float> const& pos) = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_SHADER_HPP