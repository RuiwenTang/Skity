#ifndef SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
#define SKITY_SRC_RENDER_GL_GL_DRAW_OP_H

#include <memory>
#include <skity/effect/shader.hpp>

#include "glm/glm.hpp"

namespace skity {

class GLShader;
class StencilShader;
class ColorShader;
class GLGradientShader;
class GLTextureShader;
class GLMesh;
class GLTexture;

class GLDrawOp {
 public:
  GLDrawOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
           uint32_t back_count, GLShader* shader)
      : front_start_(front_start),
        front_count_(front_count),
        back_start_(back_start),
        back_count_(back_count),
        shader_(shader),
        current_matrix_() {}
  virtual ~GLDrawOp() = default;

  void Draw(glm::mat4 const& mvp, bool has_clip = false);

  void Init();

  void UpdateCurrentMatrix(Matrix const& matrix) { current_matrix_ = matrix; }

  Matrix const& CurrentMatrix() const { return current_matrix_; }

 protected:
  inline uint32_t front_start() const { return front_start_; }
  inline uint32_t front_count() const { return front_count_; }
  inline uint32_t back_start() const { return back_start_; }
  inline uint32_t back_count() const { return back_count_; }
  inline GLShader* shader() { return shader_; }

  virtual void OnBeforeDraw(bool has_clip);
  virtual void OnAfterDraw(bool has_clip);
  virtual void OnDraw(bool has_clip) = 0;
  virtual void OnInit() = 0;

 private:
  uint32_t front_start_;
  uint32_t front_count_;
  uint32_t back_start_;
  uint32_t back_count_;
  GLShader* shader_;
  Matrix current_matrix_;
};

class GLDrawOpBuilder final {
 public:
  GLDrawOpBuilder() = default;
  ~GLDrawOpBuilder() = default;

  void UpdateStencilShader(StencilShader* shader);
  void UpdateColorShader(ColorShader* shader);
  void UpdateGradientShader(GLGradientShader* shader);
  void UpdateTextureShader(GLTextureShader* shader);
  void UpdateMesh(GLMesh* mesh);
  void UpdateFrontStart(uint32_t value);

  void UpdateFrontCount(uint32_t value);

  void UpdateBackStart(uint32_t value);

  void UpdateBackCount(uint32_t value);

  std::unique_ptr<GLDrawOp> CreateStencilOp(float stroke_width = 0.f,
                                            bool positive = true);

  std::unique_ptr<GLDrawOp> CreateColorOp(float r, float g, float b, float a);

  std::unique_ptr<GLDrawOp> CreateColorOpAA(float r, float g, float b, float a,
                                            uint32_t aa_start,
                                            uint32_t aa_count);

  std::unique_ptr<GLDrawOp> CreateGradientOp(Shader::GradientInfo* info,
                                             Shader::GradientType type);
  std::unique_ptr<GLDrawOp> CreateGradientOpAA(Shader::GradientInfo* info,
                                               Shader::GradientType type,
                                               uint32_t aa_start,
                                               uint32_t aa_count);
  std::unique_ptr<GLDrawOp> CreateTextureOp(const GLTexture* texture,
                                            Point const& p1, Point const& p2);
  std::unique_ptr<GLDrawOp> CreateTextureOpAA(const GLTexture* texture,
                                              Point const& p1, Point const& p2,
                                              uint32_t aa_start,
                                              uint32_t aa_count);

  std::unique_ptr<GLDrawOp> CreateClearStencilOp();

  std::unique_ptr<GLDrawOp> CreateDebugLineOp();

 private:
  StencilShader* stencil_shader = nullptr;
  ColorShader* color_shader = nullptr;
  GLGradientShader* gradient_shader = nullptr;
  GLTextureShader* texture_shader = nullptr;
  GLMesh* gl_mesh = nullptr;
  uint32_t front_start = 0;
  uint32_t front_count = 0;
  uint32_t back_start = 0;
  uint32_t back_count = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
