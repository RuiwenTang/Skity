#ifndef SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
#define SKITY_SRC_RENDER_GL_GL_DRAW_OP_H

#include <memory>

#include "glm/glm.hpp"

namespace skity {

class StencilShader;
class ColorShader;
class GLMesh;

class GLDrawOp {
 public:
  GLDrawOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
           uint32_t back_count)
      : front_start_(front_start),
        front_count_(front_count),
        back_start_(back_start),
        back_count_(back_count) {}
  virtual ~GLDrawOp() = default;

  void Draw();

  void Init();

 protected:
  inline uint32_t front_start() const { return front_start_; }
  inline uint32_t front_count() const { return front_count_; }
  inline uint32_t back_start() const { return back_start_; }
  inline uint32_t back_count() const { return back_count_; }

  virtual void OnBeforeDraw() = 0;
  virtual void OnAfterDraw() = 0;
  virtual void OnDraw() = 0;
  virtual void OnInit() = 0;

 private:
  uint32_t front_start_;
  uint32_t front_count_;
  uint32_t back_start_;
  uint32_t back_count_;
};

class GLDrawOpBuilder final {
 public:
  GLDrawOpBuilder() = delete;
  ~GLDrawOpBuilder() = delete;

  static void UpdateStencilShader(StencilShader* shader);
  static void UpdateColorShader(ColorShader* shader);
  static void UpdateMesh(GLMesh* mesh);
  static void UpdateMVPMatrix(glm::mat4 const& matrix);
  static void UpdateFrontStart(uint32_t value);

  static void UpdateFrontCount(uint32_t value);

  static void UpdateBackStart(uint32_t value);

  static void UpdateBackCount(uint32_t value);

  static std::unique_ptr<GLDrawOp> CreateStencilOp(float stroke_width = 0.f,
                                                   bool positive = true);

  static std::unique_ptr<GLDrawOp> CreateColorOp(float r, float g, float b,
                                                 float a);

  static std::unique_ptr<GLDrawOp> CreateClearStencilOp();

 private:
  static StencilShader* stencil_shader;
  static ColorShader* color_shader;
  static GLMesh* gl_mesh;
  static glm::mat4 mvp_matrix;
  static uint32_t front_start;
  static uint32_t front_count;
  static uint32_t back_start;
  static uint32_t back_count;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
