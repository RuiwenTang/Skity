#ifndef SKITY_SRC_RENDER_GL_GL_PATH_VISITOR_HPP
#define SKITY_SRC_RENDER_GL_GL_PATH_VISITOR_HPP

#include <skity/graphic/path.hpp>

namespace skity {
class GLVertex;
struct GLMeshRange;

/**
 * @class GLPathVisitor
 *  generate vertex for specify Path
 */
class GLPathVisitor final {
 public:
  GLPathVisitor() = delete;
  ~GLPathVisitor() = delete;
  static GLMeshRange VisitPath(Path const& path, GLVertex* gl_vertex);
};
}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_PATH_VISITOR_HPP