#ifndef SKITY_SRC_RENDER_GL_GL_DRAW_OP_H
#define SKITY_SRC_RENDER_GL_GL_DRAW_OP_H

#include <memory>

namespace skity {

class GLDrawOp {
 public:
  GLDrawOp() = default;
  virtual ~GLDrawOp() = default;

  virtual void Draw() = 0;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP_H