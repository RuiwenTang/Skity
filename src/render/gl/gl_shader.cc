#include "src/render/gl/gl_shader.hpp"

#include <iostream>
#include <string>

#include "glad/glad.h"
#include "shader.hpp"

namespace skity {

GLuint create_shader(const char* source, GLenum type) {
  GLuint shader = glCreateShader(type);

  GLint success;
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[1024];
    glGetShaderInfoLog(shader, 1204, nullptr, info_log);
    std::cerr << "shader compile error = " << info_log;
    exit(-4);
  }

  return shader;
}

GLuint create_shader_program(const char* vs_code, const char* fs_code) {
  GLuint program = glCreateProgram();
  GLint success;

  GLuint vs = create_shader(vs_code, GL_VERTEX_SHADER);
  GLuint fs = create_shader(fs_code, GL_FRAGMENT_SHADER);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (!success) {
    GLchar info_log[1024];
    glGetProgramInfoLog(program, 1024, nullptr, info_log);
    std::cerr << "program link error " << info_log << std::endl;
    exit(-5);
  }

  return program;
}

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

void GLShader::Bind() { glUseProgram(program_); }

void GLShader::UnBind() { glUseProgram(0); }

// StencilShader

void StencilShader::InitLocations() {
  mvp_location_ = glGetUniformLocation(program_, "mvp");
  stroke_width_location_ = glGetUniformLocation(program_, "stroke_radius");
}

void StencilShader::SetStrokeRadius(float width) {
  glUniform1f(stroke_width_location_, width);
}

void StencilShader::SetMVPMatrix(Matrix const& matrix) {
  glUniformMatrix4fv(mvp_location_, 1, GL_FALSE, &matrix[0][0]);
}

void ColorShader::InitLocations() {
  mvp_location_ = glGetUniformLocation(program_, "mvp");
  color_location_ = glGetUniformLocation(program_, "user_color");
}

void ColorShader::SetMVPMatrix(Matrix const& matrix) {
  SetUniform(mvp_location_, matrix);
}

void ColorShader::SetColor(float r, float g, float b, float a) {
  SetUniform(color_location_, glm::vec4{r, g, b, a});
}

std::unique_ptr<StencilShader> GLShader::CreateStencilShader() {
  std::unique_ptr<StencilShader> stencil_shader{new StencilShader()};

  std::string vs_stencil((const char*)vs_stencil_basic_glsl,
                         vs_stencil_basic_glsl_size);
  std::string fs_stencil((const char*)fs_stencil_basic_glsl,
                         fs_stencil_basic_glsl_size);

  stencil_shader->program_ =
      create_shader_program(vs_stencil.c_str(), fs_stencil.c_str());

  stencil_shader->InitLocations();

  return stencil_shader;
}

std::unique_ptr<ColorShader> GLShader::CreateColorShader() {
  std::unique_ptr<ColorShader> color_shader{new ColorShader};

  std::string vs_shader((const char*)vs_color_basic_glsl,
                        vs_color_basic_glsl_size);

  std::string fs_shader((const char*)fs_color_basic_glsl,
                        fs_color_basic_glsl_size);

  color_shader->program_ =
      create_shader_program(vs_shader.c_str(), fs_shader.c_str());

  color_shader->InitLocations();

  return color_shader;
}

}  // namespace skity