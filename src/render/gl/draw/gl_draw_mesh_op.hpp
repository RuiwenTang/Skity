#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H
#define SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H

#include "glm/glm.hpp"
#include "src/render/gl/gl_draw_op.hpp"

namespace skity {

class GLShader;
class GLMesh;

class GLDrawMeshOp : public GLDrawOp {
 public:
  GLDrawMeshOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
               uint32_t back_count, GLShader* shader, GLMesh* mesh);

 protected:
  void OnBeforeDraw(bool has_clip) override;
  void OnAfterDraw(bool has_clip) override;
  void OnDraw(bool has_clip) override;
  void OnInit() override;

  virtual void OnBeforeDrawFront();
  virtual void OnBeforeDrawBack();

  inline void UpdateDrawMode(uint32_t mode) { draw_mode_ = mode; }

 private:
  void DrawFront();
  void DrawBack();

 private:
  GLMesh* mesh_;
  uint32_t draw_mode_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H