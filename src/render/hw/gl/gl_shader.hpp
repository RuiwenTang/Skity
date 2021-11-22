#ifndef SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP
#define SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP

#include <memory>
#include <skity/effect/shader.hpp>
#include <skity/geometry/point.hpp>
#include <vector>

namespace skity {

class GLPipelineShader;

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

  static std::unique_ptr<GLPipelineShader> CreatePipelineShader();

 protected:
  GLShader() = default;

  int32_t program_ = 0;
};

class GLPipelineShader : public GLShader {
 public:
  GLPipelineShader() = default;
  ~GLPipelineShader() override = default;

  void InitLocations() override;

 private:
  int32_t mvp_location_ = -1;
  int32_t user_transform_locaion_ = -1;
  int32_t user_texture_location_ = -1;
  int32_t font_texture_location_ = -1;
  int32_t uniform_color_location_ = -1;
  int32_t stroke_width_location_ = -1;
  int32_t color_type_location_ = -1;
  int32_t gradient_count_location_ = -1;
  int32_t gradient_bound_location_ = -1;
  int32_t gradient_colors_location_ = -1;
  int32_t gradient_pos_location_ = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP
