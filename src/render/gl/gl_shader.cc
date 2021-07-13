#include "src/render/gl/gl_shader.hpp"

#include <iostream>
#include <shader.hpp>
#include <string>

#include "src/render/gl/gl_interface.hpp"

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

void GLShader::SetUniform(int32_t location, glm::vec3 const& value) {
  GL_CALL(Uniform3f, location, value[0], value[1], value[2]);
}

void GLShader::SetUniform(int32_t location, glm::vec2 const& value) {
  GL_CALL(Uniform2f, location, value[0], value[1]);
}

void GLShader::SetUniform(int32_t location, glm::mat4 const& value) {
  GL_CALL(UniformMatrix4fv, location, 1, GL_FALSE, &value[0][0]);
}

void GLShader::SetUniform(int32_t location, float value) {
  GL_CALL(Uniform1f, location, value);
}

void GLShader::Bind() { GL_CALL(UseProgram, program_); }

void GLShader::UnBind() { GL_CALL(UseProgram, 0); }

void GLShader::SetMVPMatrix(Matrix const& matrix) {
  SetUniform(mvp_location_, matrix);
}

void GLShader::InitLocations() {
  mvp_location_ = GL_CALL(GetUniformLocation, program_, "mvp");
}

// StencilShader

void StencilShader::InitLocations() {
  GLShader::InitLocations();
  stroke_width_location_ =
      GL_CALL(GetUniformLocation, program_, "stroke_radius");
}

void StencilShader::SetStrokeRadius(float width) {
  GL_CALL(Uniform1f, stroke_width_location_, width);
}

void ColorShader::InitLocations() {
  GLShader::InitLocations();
  color_location_ = GL_CALL(GetUniformLocation, program_, "user_color");
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