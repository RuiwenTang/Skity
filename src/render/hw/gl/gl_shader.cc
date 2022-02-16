
#include "src/render/hw/gl/gl_shader.hpp"

#include <array>
#include <shader.hpp>
#include <string>

#include "src/logging.hpp"
#include "src/render/hw/gl/gl_interface.hpp"

namespace skity {

#ifdef SKITY_ANDROID
#define SHADER_HEADER "#version 300 es \n precision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core \n"
#endif

GLuint create_shader(const char* source, GLenum type) {
  GLuint shader = GL_CALL(CreateShader, type);

  std::array<const char*, 2> shaders{SHADER_HEADER, source};
  GLint success;
  GL_CALL(ShaderSource, shader, shaders.size(), shaders.data(), nullptr);
  GL_CALL(CompileShader, shader);
  GL_CALL(GetShaderiv, shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[1024];
    GL_CALL(GetShaderInfoLog, shader, 1204, nullptr, info_log);
    LOG_ERROR("OpenGL shader compile error : {}", info_log);
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
    LOG_ERROR("OpenGL program link error : {}", info_log);
    exit(-5);
  }

  GL_CALL(DeleteShader, vs);
  GL_CALL(DeleteShader, fs);

  return program;
}

GLuint create_shader_program(const char* vs_code, const char* fs_code,
                             const char* gs_code) {
  GLuint program = GL_CALL(CreateProgram);
  GLint success;

  GLuint vs = create_shader(vs_code, GL_VERTEX_SHADER);
  GLuint fs = create_shader(fs_code, GL_FRAGMENT_SHADER);
  GLuint gs = create_shader(gs_code, GL_GEOMETRY_SHADER);

  GL_CALL(AttachShader, program, vs);
  GL_CALL(AttachShader, program, fs);
  GL_CALL(AttachShader, program, gs);

  GL_CALL(LinkProgram, program);
  GL_CALL(GetProgramiv, program, GL_LINK_STATUS, &success);

  if (!success) {
    GLchar info_log[1024];
    GL_CALL(GetProgramInfoLog, program, 1024, nullptr, info_log);
    LOG_ERROR("OpenGL program link error : {}", info_log);
    exit(-5);
  }

  GL_CALL(DeleteShader, vs);
  GL_CALL(DeleteShader, fs);
  GL_CALL(DeleteShader, gs);

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

void GLShader::SetUniform2i(int32_t location, const glm::ivec2& value) {
  GL_CALL(Uniform2i, location, value.x, value.y);
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

std::unique_ptr<GLPipelineShader> GLShader::CreateGSPipelineShader() {
  auto shader = std::make_unique<GLPipelineShader>();
  std::string vs_shader{(const char*)hw_gl_gs_vertex_glsl,
                        hw_gl_gs_vertex_glsl_size};
  std::string fs_shader{(const char*)hw_gl_gs_fragment_glsl,
                        hw_gl_gs_fragment_glsl_size};
  std::string gs_shader{(const char*)hw_gl_gs_geometry_glsl,
                        hw_gl_gs_geometry_glsl_size};

  shader->program_ = create_shader_program(vs_shader.c_str(), fs_shader.c_str(),
                                           gs_shader.c_str());

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
  global_alpha_location_ = GetUniformLocation("GlobalAlpha");
}

void GLPipelineShader::SetMVP(const Matrix& mvp) {
  SetUniform(mvp_location_, mvp);
}

void GLPipelineShader::SetTransformMatrix(const Matrix& matrix) {
  SetUniform(user_transform_locaion_, matrix);
}

void GLPipelineShader::SetUserTexture(int32_t unit) {
  SetUniform(user_texture_location_, unit);
}

void GLPipelineShader::SetFontTexture(int32_t unit) {
  SetUniform(font_texture_location_, unit);
}

void GLPipelineShader::SetUniformColor(const glm::vec4& color) {
  SetUniform(uniform_color_location_, color);
}

void GLPipelineShader::SetStrokeWidth(float width) {
  SetUniform(stroke_width_location_, width);
}

void GLPipelineShader::SetColorType(int32_t type) {
  SetUniform(color_type_location_, type);
}

void GLPipelineShader::SetGradientCountInfo(int32_t color_count,
                                            int32_t pos_count) {
  glm::ivec2 counts{color_count, pos_count};
  SetUniform2i(gradient_count_location_, counts);
}

void GLPipelineShader::SetGradientBoundInfo(const glm::vec4& info) {
  SetUniform(gradient_bound_location_, info);
}

void GLPipelineShader::SetGradientColors(const std::vector<glm::vec4>& colors) {
  SetUniform(gradient_colors_location_, (glm::vec4*)colors.data(),
             (int32_t)colors.size());
}

void GLPipelineShader::SetGradientPostions(const std::vector<float>& pos) {
  SetUniform(gradient_pos_location_, (float*)pos.data(), pos.size());
}

void GLPipelineShader::SetGlobalAlpha(float alpha) {
  SetUniform(global_alpha_location_, alpha);
}

}  // namespace skity
