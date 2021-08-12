#ifndef SKITY_SRC_RENDER_GL_GL_SHADER_HPP
#define SKITY_SRC_RENDER_GL_GL_SHADER_HPP

#include <memory>
#include <skity/effect/shader.hpp>
#include <skity/geometry/point.hpp>

namespace skity {

class StencilShader;
class ColorShader;
class GLGradientShader;
/**
 * @class Shader wrapper for internal use
 */
class GLShader {
 public:
  virtual ~GLShader();
  int32_t GetUniformLocation(const char* name);
  void SetUniform(int32_t location, glm::vec4 const& value);
  void SetUniform(int32_t location, glm::vec3 const& value);
  void SetUniform(int32_t location, glm::vec2 const& value);
  void SetUniform(int32_t location, glm::mat4 const& value);
  void SetUniform(int32_t location, int32_t value);
  void SetUniform(int32_t location, float value);
  void SetUniform(int32_t location, float* value, int32_t count);
  void SetUniform(int32_t location, glm::vec2* value, int32_t count);
  void SetUniform(int32_t location, glm::vec4* value, int32_t count);
  void SetUniform(int32_t location, glm::mat4* value, int32_t count);
  void SetMVPMatrix(Matrix const& mvp);
  void Bind();

  void UnBind();

  virtual void InitLocations();

  static std::unique_ptr<StencilShader> CreateStencilShader();

  static std::unique_ptr<ColorShader> CreateColorShader();

  static std::unique_ptr<GLGradientShader> CreateGradientShader();

 protected:
  GLShader() = default;

  int32_t program_ = 0;
  int32_t mvp_location_ = -1;
};

class StencilShader : public GLShader {
 public:
  StencilShader() = default;
  ~StencilShader() override = default;
  void SetStrokeRadius(float width);

  void InitLocations() override;

 private:
  int32_t stroke_width_location_ = -1;
};

class ColorShader : public GLShader {
 public:
  ColorShader() = default;
  ~ColorShader() override = default;

  void InitLocations() override;
  void SetColor(float r, float g, float b, float a);

 private:
  int32_t color_location_ = -1;
};

class GLGradientShader : public GLShader {
 public:
  GLGradientShader() = default;
  ~GLGradientShader() override = default;

  void InitLocations() override;
  void SetMatrixs(const Matrix matrix[2]);
  void SetPoints(Point const& p1, Point const& p2);
  void SetRadius(float r1, float r2);
  void SetColorCount(int32_t value);
  void SetGradientType(int32_t type);
  void SetStopCount(int32_t value);
  void SetColors(std::vector<Vec4> const& colors);
  void SetStops(std::vector<float> const& stops);

 private:
  int32_t matrixs_location_ = -1;
  int32_t points_location_ = -1;
  int32_t radius_location_ = -1;
  int32_t color_count_location_ = -1;
  int32_t gradient_type_location_ = -1;
  int32_t stop_count_location_ = -1;
  int32_t colors_location_ = -1;
  int32_t stops_location_ = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_SHADER_HPP
