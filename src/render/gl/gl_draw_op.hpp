#ifndef SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
#define SKITY_SRC_RENDER_GL_GL_DRAW_OP_H

#include <memory>

#include "glm/glm.hpp"

namespace skity {

class GLShader;
class StencilShader;
class ColorShader;
class GLMesh;

class GLDrawOp {
 public:
  GLDrawOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
           uint32_t back_count, GLShader* shader)
      : front_start_(front_start),
        front_count_(front_count),
        back_start_(back_start),
        back_count_(back_count),
        shader_(shader) {}
  virtual ~GLDrawOp() = default;

  void Draw(glm::mat4 const& mvp, bool has_clip = false);

  void Init();

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
};

class GLDrawOpBuilder final {
 public:
  GLDrawOpBuilder() = default;
  ~GLDrawOpBuilder() = default;

  void UpdateStencilShader(StencilShader* shader);
  void UpdateColorShader(ColorShader* shader);
  void UpdateMesh(GLMesh* mesh);
  void UpdateFrontStart(uint32_t value);

  void UpdateFrontCount(uint32_t value);

  void UpdateBackStart(uint32_t value);

  void UpdateBackCount(uint32_t value);

  std::unique_ptr<GLDrawOp> CreateStencilOp(float stroke_width = 0.f,
                                            bool positive = true);

  std::unique_ptr<GLDrawOp> CreateColorOp(float r, float g, float b, float a);

  std::unique_ptr<GLDrawOp> CreateClearStencilOp();

 private:
  StencilShader* stencil_shader = nullptr;
  ColorShader* color_shader = nullptr;
  GLMesh* gl_mesh = nullptr;
  uint32_t front_start = 0;
  uint32_t front_count = 0;
  uint32_t back_start = 0;
  uint32_t back_count = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
