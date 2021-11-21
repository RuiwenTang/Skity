#ifndef SKITY_SRC_RENDER_HW_HW_SHADER_HPP
#define SKITY_SRC_RENDER_HW_HW_SHADER_HPP

#include <glm/glm.hpp>
#include <skity/graphic/color.hpp>
#include <vector>

namespace skity {

enum HWPipelineMode {
  kStencil = 0,
  kUniformColor = 1,
  kImageTexture = 2,
  kLinearGradient = 3,
  kRadialGradient = 4,
};

enum class HWStencilOp {
  INC_WRAP,
  DESC_WRAP,
  KEEP,
  REPLACE,
};

enum class HWStencilFunc {
  EQUAL,
  NOT_EQUAL,
  LESS,
  ALWAYS,
};

class HWPipeline {
 public:
  virtual ~HWPipeline() = default;

  virtual void Init() = 0;

  virtual void Bind() = 0;

  virtual void UnBind() = 0;

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
   * @brief Upload PipelineMode to GPU shader
   *
   */
  virtual void SetPipelineMode(HWPipelineMode mode) = 0;

  /**
   * @brief Upload StrokeWidth to GPU shader
   */
  virtual void SetStrokeWidth(float width) = 0;
  /**
   * @brief vec4 info property
   *        [p1.x, p1.y, p2.x, p2.y]
   *
   * @param info
   */
  virtual void SetGradientBoundInfo(glm::vec4 const& info) = 0;

  /**
   * @brief Upload gradient count info to GPU shader
   *
   */
  virtual void SetGradientCountInfo(int32_t color_count, int32_t pos_count) = 0;

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

  /**
   * @brief Upload vertex buffer data to GPU
   *
   * @param data pointer to buffer data
   * @param data_size size of buffer data
   */
  virtual void UploadVertexBuffer(void* data, size_t data_size) = 0;

  virtual void UploadIndexBuffer(void* data, size_t data_size) = 0;

  virtual void EnableStencilTest() = 0;

  virtual void DisableStencilTest() = 0;

  virtual void UpdateStencilMask(uint8_t write_mask, uint8_t compile_mask) = 0;

  virtual void UpdateStencilOp(HWStencilOp op) = 0;

  virtual void UpdateStencilFunc(HWStencilFunc func, uint32_t value) = 0;

  virtual void DrawIndex(uint32_t start, uint32_t count) = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_SHADER_HPP
