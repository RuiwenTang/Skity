#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H
#define SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H

#include "src/render/gl/gl_draw_op.hpp"

namespace skity {

class GLShader;
class GLMesh;

class GLDrawMeshOp : public GLDrawOp {
 public:
  GLDrawMeshOp(uint32_t front_start, uint32_t front_count, uint32_t back_start,
               uint32_t back_count, GLShader* shader, GLMesh* mesh);

 protected:
  void OnBeforeDraw() override;
  void OnAfterDraw() override;
  void OnDraw() override;
  void OnInit() override;

  virtual void OnBeforeDrawFront();
  virtual void OnBeforeDrawBack();

 private:
  void DrawFront();
  void DrawBack();

 private:
  GLShader* shader_;
  GLMesh* mesh_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_H