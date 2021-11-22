
#include "src/render/hw/gl/gl_shader.hpp"

#include <array>
#include <iostream>
#include <shader.hpp>
#include <string>

#include "src/render/hw/gl/gl_interface.hpp"

namespace skity {

GLuint create_shader(const char* source, GLenum type) {
  GLuint shader = GL_CALL(CreateShader, type);

  GLint success;
  GL_CALL(ShaderSource, shader, 1, &source, nullptr);
  GL_CALL(CompileShader, shader);
  GL_CALL(GetShaderiv, shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[1024];
    GL_CALL(GetShaderInfoLog, shader, 1204, nullptr, info_log);
    std::cerr << "shader compile error = " << info_log;
    exit(-4);
  }

  return shader;
}

GLuint create_shader_program(const char* vs_code, const char* fs_code) {
  GLuint program = GL_CALL(CreateProgram);
  GLint success;

  GLuint vs = create_shader(vs_code, GL_VERTEX_SHADER);
  GLuint fs = create_shader(fs_code, GL_FRAGMENT_SHADER);

  GL_CALL(AttachShader, program, vs);
  GL_CALL(AttachShader, program, fs);
  GL_CALL(LinkProgram, program);
  GL_CALL(GetProgramiv, program, GL_LINK_STATUS, &success);

  if (!success) {
    GLchar info_log[1024];
    GL_CALL(GetProgramInfoLog, program, 1024, nullptr, info_log);
    std::cerr << "program link error " << info_log << std::endl;
    exit(-5);
  }

  return program;
}

GLShader::~GLShader() {
  if (program_) {
    GL_CALL(DeleteProgram, program_);
  }
}

int32_t GLShader::GetUniformLocation(const char* name) {
  return GL_CALL(GetUniformLocation, program_, name);
}

void GLShader::SetUniform(int32_t location, glm::vec4 const& value) {
  GL_CALL(Uniform4f, location, value[0], value[1], value[2], value[3]);
}

void GLShader::SetUniform4i(int32_t location, const glm::ivec4& value) {
  GL_CALL(Uniform4i, location, value[0], value[1], value[2], value[3]);
}

void GLShader::SetUniform(int32_t location, glm::vec3 const& value) {
  GL_CALL(Uniform3f, location, value[0], value[1], value[2]);
}

void GLShader::SetUniform(int32_t location, glm::vec2 const& value) {
  GL_CALL(Uniform2f, location, value[0], value[1]);
}

void GLShader::SetUniform(int32_t location, glm::mat4 const& value) {
  GL_CALL(UniformMatrix4fv, location, 1, GL_FALSE, &value[0][0]);
}

void GLShader::SetUniform(int32_t location, int32_t value) {
  GL_CALL(Uniform1i, location, value);
}

void GLShader::SetUniform(int32_t location, float value) {
  GL_CALL(Uniform1f, location, value);
}

void GLShader::SetUniform(int32_t location, float* value, int32_t count) {
  GL_CALL(Uniform1fv, location, count, value);
}

void GLShader::SetUniform(int32_t location, glm::vec2* value, int32_t count) {
  GL_CALL(Uniform2fv, location, count, (float*)value);
}

void GLShader::SetUniform(int32_t location, glm::vec4* value, int32_t count) {
  GL_CALL(Uniform4fv, location, count, (float*)value);
}

void GLShader::SetUniform(int32_t location, glm::mat4* value, int32_t count) {
  GL_CALL(UniformMatrix4fv, location, count, GL_FALSE, (float*)value);
}

void GLShader::Bind() { GL_CALL(UseProgram, program_); }

void GLShader::UnBind() { GL_CALL(UseProgram, 0); }

void GLShader::InitLocations() {}

std::unique_ptr<GLPipelineShader> GLShader::CreatePipelineShader() {
  auto shader = std::make_unique<GLPipelineShader>();
  std::string vs_shader{(const char*)hw_gl_vertex_glsl, hw_gl_vertex_glsl_size};
  std::string fs_shader{(const char*)hw_gl_fragment_glsl,
                        hw_gl_fragment_glsl_size};
  shader->program_ =
      create_shader_program(vs_shader.c_str(), fs_shader.c_str());

  shader->InitLocations();

  return shader;
}

void GLPipelineShader::InitLocations() {
  GLShader::InitLocations();

  mvp_location_ = GetUniformLocation("mvp");
  user_transform_locaion_ = GetUniformLocation("UserTransform");
  user_texture_location_ = GetUniformLocation("UserTexture");
  font_texture_location_ = GetUniformLocation("FontTexture");
  uniform_color_location_ = GetUniformLocation("UserColor");
  stroke_width_location_ = GetUniformLocation("StrokeWidth");
  color_type_location_ = GetUniformLocation("ColorType");
  gradient_count_location_ = GetUniformLocation("GradientCounts");
  gradient_bound_location_ = GetUniformLocation("GradientBounds");
  gradient_colors_location_ = GetUniformLocation("GradientColors");
  gradient_pos_location_ = GetUniformLocation("GradientStops");
}

}  // namespace skity
