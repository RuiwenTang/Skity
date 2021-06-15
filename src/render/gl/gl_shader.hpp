#ifndef SKITY_SRC_RENDER_GL_GL_SHADER_HPP
#define SKITY_SRC_RENDER_GL_GL_SHADER_HPP

#include <memory>
#include <skity/geometry/point.hpp>

namespace skity {
/**
 * @class Shader wrapper for internal use
 */
class GLShader final {
 public:
  ~GLShader();
  int32_t GetUniformLocation(const char* name);
  void SetUniform(int32_t location, glm::vec4 const& value);
  void SetUniform(int32_t location, glm::vec3 const& value);
  void SetUniform(int32_t location, glm::vec2 const& value);
  void SetUniform(int32_t location, glm::mat4 const& value);
  void SetUniform(int32_t location, float value);
  static std::unique_ptr<GLShader> CreateStencilShader();

 private:
  GLShader() = default;
  int32_t program_ = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_SHADER_HPP