#include "src/render/gl/gl_shader.hpp"

#include <array>
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

void GLGradientShader::InitLocations() {
  GLShader::InitLocations();
  matrixs_location_ = GL_CALL(GetUniformLocation, program_, "matrixs");
  points_location_ = GL_CALL(GetUniformLocation, program_, "points");
  radius_location_ = GL_CALL(GetUniformLocation, program_, "radius");
  color_count_location_ = GL_CALL(GetUniformLocation, program_, "colorCount");
  gradient_type_location_ =
      GL_CALL(GetUniformLocation, program_, "gradientType");
  stop_count_location_ = GL_CALL(GetUniformLocation, program_, "stopCount");
  colors_location_ = GL_CALL(GetUniformLocation, program_, "colors");
  stops_location_ = GL_CALL(GetUniformLocation, program_, "colorStops");
  premul_alpha_location_ = GL_CALL(GetUniformLocation, program_, "premulAlpha");
}

void GLGradientShader::SetMatrixs(const Matrix matrix[2]) {
  SetUniform(matrixs_location_, (Matrix*)matrix, 2);
}

void GLGradientShader::SetPoints(Point const& p1, Point const& p2) {
  std::array<glm::vec2, 2> pts{};
  pts[0].x = p1.x;
  pts[0].y = p1.y;
  pts[1].x = p2.x;
  pts[1].y = p2.y;

  SetUniform(points_location_, pts.data(), 2);
}

void GLGradientShader::SetRadius(float r1, float r2) {
  std::array<float, 2> pts{};
  pts[0] = r1;
  pts[1] = r2;

  SetUniform(radius_location_, pts.data(), 2);
}

void GLGradientShader::SetColorCount(int32_t value) {
  SetUniform(color_count_location_, value);
}

void GLGradientShader::SetGradientType(int32_t type) {
  SetUniform(gradient_type_location_, type);
}

void GLGradientShader::SetStopCount(int32_t value) {
  SetUniform(stop_count_location_, value);
}

void GLGradientShader::SetColors(std::vector<Vec4> const& colors) {
  SetUniform(colors_location_, (Vec4*)colors.data(), colors.size());
}

void GLGradientShader::SetStops(std::vector<float> const& stops) {
  SetUniform(stops_location_, (float*)stops.data(), stops.size());
}

void GLGradientShader::SetPremulAlphaFlag(int32_t value) {
  SetUniform(premul_alpha_location_, value);
}

void GLTextureShader::InitLocations() {
  GLShader::InitLocations();
  matrixs_location_ = GL_CALL(GetUniformLocation, program_, "matrixs");
  bounds_location_ = GL_CALL(GetUniformLocation, program_, "bounds");
  texture_location_ = GL_CALL(GetUniformLocation, program_, "ourTexture");
}

void GLTextureShader::SetTextureChannel(int32_t channel) {
  SetUniform(texture_location_, channel);
}

void GLTextureShader::SetMatrixs(const Matrix matrix[2]) {
  SetUniform(matrixs_location_, (Matrix*)matrix, 2);
}

void GLTextureShader::SetBounds(Point const& p1, Point const& p2) {
  std::array<glm::vec2, 2> pts{};
  pts[0].x = p1.x;
  pts[0].y = p1.y;
  pts[1].x = p2.x;
  pts[1].y = p2.y;

  SetUniform(bounds_location_, pts.data(), 2);
}

void GLUniverseShader::InitLocations() {
  GLShader::InitLocations();
  user_color_location_ = GL_CALL(GetUniformLocation, program_, "UserColor");
  user_data1_location_ = GL_CALL(GetUniformLocation, program_, "UserData1");
  user_data2_location_ = GL_CALL(GetUniformLocation, program_, "UserData2");
  user_data3_location_ = GL_CALL(GetUniformLocation, program_, "UserData3");
  user_data4_location_ = GL_CALL(GetUniformLocation, program_, "UserData4");
  user_texture_location_ = GL_CALL(GetUniformLocation, program_, "UserTexture");
  gradient_colors_location_ =
      GL_CALL(GetUniformLocation, program_, "GradientColors");
  gradient_stops_location_ =
      GL_CALL(GetUniformLocation, program_, "GradientStops");
}

void GLUniverseShader::SetUserColor(const Vec4& value) {
  this->SetUniform(user_color_location_, value);
}

void GLUniverseShader::SetUserData1(const glm::ivec4& value) {
  this->SetUniform4i(user_data1_location_, value);
}

void GLUniverseShader::SetUserData2(glm::vec4 const& value) {
  this->SetUniform(user_data2_location_, value);
}

void GLUniverseShader::SetUserData3(const glm::vec4& value) {
  this->SetUniform(user_data3_location_, value);
}

void GLUniverseShader::SetUserData4(const glm::vec4& value) {
  this->SetUniform(user_data4_location_, value);
}

void GLUniverseShader::SetUserTexture(int32_t value) {
  this->SetUniform(user_texture_location_, value);
}

void GLUniverseShader::SetGradientColors(const std::vector<glm::vec4>& colors) {
  SetUniform(gradient_colors_location_, (glm::vec4*)colors.data(),
             colors.size());
}

void GLUniverseShader::SetGradientStops(const std::vector<float>& stops) {
  SetUniform(gradient_stops_location_, (float*)stops.data(), stops.size());
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

std::unique_ptr<GLGradientShader> GLShader::CreateGradientShader() {
  std::unique_ptr<GLGradientShader> gradient_shader{new GLGradientShader};

  std::string vs_shader((const char*)vs_gradient_basic_glsl,
                        vs_gradient_basic_glsl_size);
  std::string fs_shader((const char*)fs_gradient_basic_glsl,
                        fs_gradient_basic_glsl_size);

  gradient_shader->program_ =
      create_shader_program(vs_shader.c_str(), fs_shader.c_str());

  gradient_shader->InitLocations();

  return gradient_shader;
}

std::unique_ptr<GLTextureShader> GLShader::CreateTextureShader() {
  std::unique_ptr<GLTextureShader> texture_shader{new GLTextureShader};

  std::string vs_shader((const char*)vs_bitmap_basic_glsl,
                        vs_bitmap_basic_glsl_size);

  std::string fs_shader((const char*)fs_bitmap_basic_glsl,
                        fs_bitmap_basic_glsl_size);

  texture_shader->program_ =
      create_shader_program(vs_shader.c_str(), fs_shader.c_str());

  texture_shader->InitLocations();

  return texture_shader;
}

std::unique_ptr<GLUniverseShader> GLShader::CreateUniverseShader() {
  std::unique_ptr<GLUniverseShader> shader{new GLUniverseShader};

  std::string vs_shader((const char*)universe_vertex_glsl,
                        universe_vertex_glsl_size);
  std::string fs_shader((const char*)universe_fragment_glsl,
                        universe_fragment_glsl_size);

  shader->program_ =
      create_shader_program(vs_shader.c_str(), fs_shader.c_str());

  shader->InitLocations();

  return shader;
}

}  // namespace skity
