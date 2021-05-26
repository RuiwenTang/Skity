#include "src/render/gl/gl_stroke.hpp"

#include "src/render/gl/gl_path_visitor.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLStroke::GLStroke(Paint const& paint) : stroke_(paint) {}

void GLStroke::strokePath(Path const& path, GLVertex* gl_vertex) {
  Path result;
  stroke_.strokePath(path, &result);

  GLPathVisitor::VisitPath(result, gl_vertex);
}

}  // namespace skity