#include "src/render/gl/draw/gl_draw_debug_line_op.hpp"

#include <glad/glad.h>

#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLDrawDebugLineOp::GLDrawDebugLineOp(uint32_t front_start, uint32_t front_count,
                                     uint32_t back_start, uint32_t back_count,
                                     ColorShader* shader, GLMesh* mesh)
    : GLDrawMeshOp(front_start, front_count, back_start, back_count, shader,
                   mesh),
      shader_(shader) {}

void GLDrawDebugLineOp::OnInit() {
  GLDrawMeshOp::OnInit();
  UpdateDrawMode(GL_LINE_LOOP);
}

}  // namespace skity