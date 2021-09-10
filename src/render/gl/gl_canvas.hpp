#ifndef SKITY_SRC_RENDER_GL_GL_CANVAS_HPP
#define SKITY_SRC_RENDER_GL_GL_CANVAS_HPP

#include <memory>
#include <skity/render/canvas.hpp>
#include <vector>

#include "src/render/gl/gl_draw_op.hpp"
#include "src/render/gl/gl_glyph_raster_cache.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_texture.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

class GLCanvasState final {
  struct State {
    Matrix matrix = {};
    Matrix clip_matrix = {};
    GLMeshRange clip_path_range;
    bool has_clip = false;
  };

 public:
  GLCanvasState(Matrix mvp, GLMesh* mesh, StencilShader* shader,
                ColorShader* color_shader);
  ~GLCanvasState() = default;

  void UpdateCurrentMatrix(Matrix const& mvp);
  void UpdateCurrentClipPathRange(GLMeshRange const& range);
  Matrix CurrentMatrix();

  void DoClipPath(uint32_t stack_depth);

  void UnDoClipPath();

  int32_t CurrentStackDepth() const;

  void PushStack();

  void PopStack(int32_t target_stack_depth);
  void PopStack();

  bool HasClip();

  bool CurrentHasClipPath();

 private:
  void DrawFront(GLMeshRange const& range);
  void DrawBack(GLMeshRange const& range);

 private:
  std::vector<State> state_stack_;
  std::vector<bool> clip_stack_;
  Matrix mvp_;
  GLMesh* mesh_;
  StencilShader* shader_;
  ColorShader* color_shader_;
};

class GLCanvas : public Canvas {
 public:
  explicit GLCanvas(Matrix const& mvp, float width, float height);
  ~GLCanvas() override = default;

 protected:
  void onClipPath(Path const& path, ClipOp op) override;

  void onDrawPath(Path const& path, Paint const& paint) override;

  void onDrawGlyphs(std::vector<GlyphInfo> const& glyphs,
                    const Typeface* typeface, Paint const& paint) override;

  void onFlush() override;

  void onSave() override;

  void onRestore() override;

  void onTranslate(float dx, float dy) override;

  void onScale(float sx, float sy) override;

  void onRotate(float degree) override;

  void onRotate(float degree, float px, float py) override;

  void onConcat(Matrix const& matrix) override;

  void onUpdateViewport(uint32_t width, uint32_t height) override;

  uint32_t onGetWidth() const override;
  uint32_t onGetHeight() const override;

 private:
  void Init();
  void InitShader();
  void InitMesh();
  void InitDrawOpBuilder();
  void UpdateDrawOpBuilder(GLMeshRange const& range);
  std::unique_ptr<GLDrawOp> GenerateColorOp(Paint const& paint, bool fill,
                                            skity::Rect const& bounds,
                                            GLMeshRange* aa_range);

 private:
  float width_;
  float height_;
  std::unique_ptr<StencilShader> stencil_shader_ = {};
  std::unique_ptr<ColorShader> color_shader_ = {};
  std::unique_ptr<GLGradientShader> gradient_shader_ = {};
  std::unique_ptr<GLTextureShader> texture_shader_ = {};
  std::unique_ptr<GLVertex> vertex_ = {};
  std::unique_ptr<GLMesh> mesh_ = {};
  GLDrawOpBuilder draw_op_builder_ = {};
  std::vector<std::unique_ptr<GLDrawOp>> draw_ops_ = {};
  GLVertex gl_vertex_ = {};
  Matrix mvp_;
  std::unique_ptr<GLCanvasState> state_;
  std::unique_ptr<GLTextureManager> texture_manager_;
  std::unique_ptr<GLGlyphRasterCache> glyph_raster_cache_;
};
}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_CANVAS_HPP