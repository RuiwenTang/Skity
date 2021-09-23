
#include "src/render/gl/gl_fill2.hpp"

namespace skity {

GLFill2::GLFill2(const Paint &paint, GLVertex2 *gl_vertex)
    : GLPathVisitor(paint, gl_vertex) {}

void GLFill2::HandleMoveTo(const Point &pt) {}

void GLFill2::HandleLineTo(const Point &from, const Point &to) {}

void GLFill2::HandleQuadTo(const Point &from, const Point &control,
                           const Point &end) {}

void GLFill2::HandleClose() {}

void GLFill2::HandleFinish(GLMeshRange *range) {}

}  // namespace skity
