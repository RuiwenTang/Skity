#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_HPP
#define SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_HPP

#include "src/render/gl/draw/gl_draw_mesh_op.hpp"

namespace skity {

class GLDrawMeshOpAA : public GLDrawMeshOp {
 public:
  GLDrawMeshOpAA(uint32_t front_start, uint32_t front_count,
                 uint32_t back_start, uint32_t back_count, uint32_t aa_start,
                 uint32_t aa_count, GLShader* shader, GLMesh* mesh);
  ~GLDrawMeshOpAA() override = default;

 protected:
  void OnDraw(bool has_clip) override;

  void OnBeforeDraw(bool has_clip) override;
  void OnAfterDraw(bool has_clip) override;
  virtual void OnBeforeDrawAAOutline(bool has_clip);

 private:
  void DrawAAOutline();

 private:
  GLMesh* mesh_;
  uint32_t aa_start_;
  uint32_t aa_count_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_MESH_OP_HPP