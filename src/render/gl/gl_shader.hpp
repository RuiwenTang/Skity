#ifndef SKITY_SRC_RENDER_GL_GL_SHADER_HPP
#define SKITY_SRC_RENDER_GL_GL_SHADER_HPP

#include <memory>
#include <skity/geometry/point.hpp>

namespace skity {

class StencilShader;
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
  void SetUniform(int32_t location, float value);

  void Bind();

  void UnBind();
  static std::unique_ptr<StencilShader> CreateStencilShader();

 protected:
  GLShader() = default;

  int32_t program_ = 0;
};

class StencilShader : public GLShader {
 public:
  StencilShader() = default;
  ~StencilShader() override = default;
  void SetStrokeRadius(float width);
  void SetMVPMatrix(Matrix const& mvp);

  void InitLocations();

 private:
  int32_t stroke_width_location_ = -1;
  int32_t mvp_location_ = -1;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_SHADER_HPP