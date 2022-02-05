#ifndef SKITY_SRC_RENDER_HW_HW_SHADER_HPP
#define SKITY_SRC_RENDER_HW_HW_SHADER_HPP

#include <glm/glm.hpp>
#include <skity/graphic/color.hpp>
#include <vector>

namespace skity {

enum HWPipelineColorMode {
  kStencil = 0,
  kUniformColor = 1,
  kImageTexture = 2,
  kLinearGradient = 3,
  kRadialGradient = 4,
  kFBOTexture = 5,
  kHorizontalBlur = 6,
  kVerticalBlur = 7,
  kSolidBlurMix = 8,
  kOuterBlurMix = 9,
  kInnerBlurMix = 10,
};

enum class HWStencilOp {
  INCR_WRAP,
  DECR_WRAP,
  KEEP,
  REPLACE,
};

enum class HWStencilFunc {
  EQUAL,
  NOT_EQUAL,
  LESS,
  LESS_OR_EQUAL,
  GREAT_OR_EQUAL,
  ALWAYS,
};

class HWTexture;
class HWRenderTarget;

class HWRenderer {
 public:
  virtual ~HWRenderer() = default;

  virtual void Init() = 0;

  virtual void Destroy() = 0;

  virtual void Bind() = 0;

  virtual void UnBind() = 0;

  /**
   * @brief Upload projection matrix to GPU shader
   *
   * @param mvp
   */
  virtual void SetViewProjectionMatrix(glm::mat4 const& mvp) {
    mvp_matrix_ = mvp;
  }

  glm::mat4 GetMVPMatrix() const { return mvp_matrix_; }

  /**
   * @brief Upload transform matrix to GPU shader
   *
   * @param matrix
   */
  virtual void SetModelMatrix(glm::mat4 const& matrix) {
    model_matrix_ = matrix;
  }

  glm::mat4 GetModelMatrix() const { return model_matrix_; }

  /**
   * @brief Upload PipelineMode to GPU shader
   *
   */
  virtual void SetPipelineColorMode(HWPipelineColorMode mode) = 0;

  /**
   * @brief Upload StrokeWidth to GPU shader
   */
  virtual void SetStrokeWidth(float width) = 0;

  /**
   * @brief Upload uniform color to GPU shader
   *
   */
  virtual void SetUniformColor(glm::vec4 const& color) = 0;

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
  virtual void SetGradientColors(std::vector<Color4f> const& colors) = 0;

  /**
   * @brief Upload gradient positions to GPU shader
   *
   * @param pos
   */
  virtual void SetGradientPositions(std::vector<float> const& pos) = 0;

  /**
   * @brief Upload vertex buffer data to GPU
   *
   * @param data pointer to buffer data
   * @param data_size size of buffer data
   */
  virtual void UploadVertexBuffer(void* data, size_t data_size) = 0;

  virtual void UploadIndexBuffer(void* data, size_t data_size) = 0;

  virtual void SetGlobalAlpha(float alpha) = 0;

  virtual void EnableStencilTest() = 0;

  virtual void DisableStencilTest() = 0;

  virtual void EnableColorOutput() = 0;

  virtual void DisableColorOutput() = 0;

  virtual void UpdateStencilMask(uint8_t write_mask) = 0;

  virtual void UpdateStencilOp(HWStencilOp op) = 0;

  virtual void UpdateStencilFunc(HWStencilFunc func, uint32_t value,
                                 uint32_t compare_mask) = 0;

  virtual void DrawIndex(uint32_t start, uint32_t count) = 0;

  virtual void BindTexture(HWTexture* texture, uint32_t slot) = 0;

  virtual void BindRenderTarget(HWRenderTarget* render_target) = 0;

  virtual void UnBindRenderTarget(HWRenderTarget* render_target) = 0;

 private:
  glm::mat4 mvp_matrix_ = {};
  glm::mat4 model_matrix_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_SHADER_HPP
