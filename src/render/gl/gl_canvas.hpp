#ifndef SKITY_SRC_RENDER_GL_GL_CANVAS_HPP
#define SKITY_SRC_RENDER_GL_GL_CANVAS_HPP

#include <skity/render/canvas.hpp>
#include <vector>

#include "src/render/gl/gl_draw_op.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {
class GLCanvas : public Canvas {
 public:
  explicit GLCanvas(Matrix const& mvp);
  ~GLCanvas() override = default;

 protected:
  void onClipPath(Path const& path, ClipOp op) override;

  void onDrawPath(Path const& path, Paint const& paint) override;

  void onFlush() override;

  void onSave() override;

  void onRestore() override;

 private:
  void Init();
  void InitShader();
  void InitMesh();
  void InitDrawOpBuilder();
  void UpdateDrawOpBuilder(GLMeshRange const& range);

 private:
  std::unique_ptr<StencilShader> stencil_shader_ = {};
  std::unique_ptr<ColorShader> color_shader_ = {};
  std::unique_ptr<GLVertex> vertex_ = {};
  std::unique_ptr<GLMesh> mesh_ = {};
  GLDrawOpBuilder draw_op_builder_ = {};
  std::vector<std::unique_ptr<GLDrawOp>> draw_ops_ = {};
  GLVertex gl_vertex_ = {};
  Matrix mvp_;
};
}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_CANVAS_HPP