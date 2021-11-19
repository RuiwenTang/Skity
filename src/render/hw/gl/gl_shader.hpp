#ifndef SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP
#define SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP

#include <memory>
#include <skity/effect/shader.hpp>
#include <skity/geometry/point.hpp>
#include <vector>

namespace skity {

class GLColorStencilShader;

/**
 * @class Shader wrapper for internal use
 */
class GLShader {
 public:
  virtual ~GLShader();
  int32_t GetUniformLocation(const char* name);
  void SetUniform(int32_t location, glm::vec4 const& value);
  void SetUniform4i(int32_t location, glm::ivec4 const& value);
  void SetUniform(int32_t location, glm::vec3 const& value);
  void SetUniform(int32_t location, glm::vec2 const& value);
  void SetUniform(int32_t location, glm::mat4 const& value);
  void SetUniform(int32_t location, int32_t value);
  void SetUniform(int32_t location, float value);
  void SetUniform(int32_t location, float* value, int32_t count);
  void SetUniform(int32_t location, glm::vec2* value, int32_t count);
  void SetUniform(int32_t location, glm::vec4* value, int32_t count);
  void SetUniform(int32_t location, glm::mat4* value, int32_t count);
  void Bind();

  void UnBind();

  virtual void InitLocations();

  static std::unique_ptr<GLColorStencilShader> CreateColorStencilShader();

 protected:
  GLShader() = default;

  int32_t program_ = 0;
};

class GLColorStencilShader : public GLShader {
 public:
  ~GLColorStencilShader() override = default;

  void SetProjectionMatrix(glm::mat4 const& matrix);
  void SetTransformMatrix(glm::mat4 const& matrix);
  void SetVecIInfo1(glm::ivec4 const& info);
  void SetVecIInfo2(glm::ivec4 const& info);
  void SetVecFInfo1(glm::vec4 const& info);
  void SetVecFInfo2(glm::vec4 const& info);

  void InitLocations() override;

 private:
  int32_t mvp_location_ = -1;
  int32_t transform_location_ = -1;
  int32_t veci_info1_location_ = -1;
  int32_t veci_info2_location_ = -1;
  int32_t vecf_info1_location_ = -1;
  int32_t vecf_info2_location_ = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP
