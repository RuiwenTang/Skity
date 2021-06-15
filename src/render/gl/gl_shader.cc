#include "src/render/gl/gl_shader.hpp"

#include "glad/glad.h"
#include "shader.hpp"

namespace skity {

GLShader::~GLShader() {
  if (program_) {
    glDeleteProgram(program_);
  }
}

int32_t GLShader::GetUniformLocation(const char* name) {
  return glGetUniformLocation(program_, name);
}

void GLShader::SetUniform(int32_t location, glm::vec4 const& value) {
  glUniform4f(location, value[0], value[1], value[2], value[3]);
}

void GLShader::SetUniform(int32_t location, glm::vec3 const& value) {
  glUniform3f(location, value[0], value[1], value[2]);
}

void GLShader::SetUniform(int32_t location, glm::vec2 const& value) {
  glUniform2f(location, value[0], value[1]);
}

void GLShader::SetUniform(int32_t location, glm::mat4 const& value) {
  glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

void GLShader::SetUniform(int32_t location, float value) {
  glUniform1f(location, value);
}

}  // namespace skity