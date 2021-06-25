#include <skity/render/canvas.hpp>

#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {
class GLCanvas : public Canvas {
 public:
  GLCanvas();
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

 private:
  std::unique_ptr<StencilShader> stencil_shader_;
  std::unique_ptr<GLVertex> vertex_;
  std::unique_ptr<GLMesh> mesh_;
};
}  // namespace skity