#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_DEBUG_LINE_OP_HPP
#define SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_DEBUG_LINE_OP_HPP

#include "gl_draw_mesh_op.hpp"

namespace skity {

class ColorShader;

class GLDrawDebugLineOp : public GLDrawMeshOp {
 public:
  GLDrawDebugLineOp(uint32_t front_start, uint32_t front_count,
                    uint32_t back_start, uint32_t back_count,
                    ColorShader* shader, GLMesh* mesh);

  ~GLDrawDebugLineOp() override = default;

 protected:
  void OnBeforeDraw(bool has_clip) override;
  void OnAfterDraw(bool has_clip) override;

  void OnInit() override;

 private:
  ColorShader* shader_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_DRAW_DEBUG_LINE_OP_HPP