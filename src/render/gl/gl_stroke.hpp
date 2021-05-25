#ifndef SKITY_SRC_RENDER_GL_GL_STROKE_HPP
#define SKITY_SRC_RENDER_GL_GL_STROKE_HPP

#include "src/render/stroke.hpp"

namespace skity {

class GLVertex;

/**
 * @class GLStroke
 *  Used to generate Mesh for path stroke
 *  @todo generate vertex during stroke, not at the end
 */
class GLStroke {
 public:
  explicit GLStroke(Paint const& paint);
  ~GLStroke() = default;

  void strokePath(Path const& path, GLVertex* gl_vertex);

 private:
 private:
  Stroke stroke_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_STROKE_HPP