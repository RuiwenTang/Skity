#ifndef SKITY_SRC_RENDER_GL_GL_PATH_STROKE
#define SKITY_SRC_RENDER_GL_GL_PATH_STROKE

#if 0

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {
class GLVertex;
/**
 * @class GLPathStroke
 *
 *  Helper class to generate stroke path mesh
 */
class GLPathStroke {
 public:
  GLPathStroke();
  explicit GLPathStroke(Paint const& paint);

  Paint::Cap getCap() const { return cap_; }
  Paint::Join getJoin() const { return join_; }

  void setCap(Paint::Cap cap) { cap_ = cap; }
  void setJoin(Paint::Join join) { join_ = join; }
  void setMiterLimit(float limit) { miter_limit_ = limit; }
  void setWidth(float width) { width_ = width; }
  void setResScale(float rs) { res_scale_ = rs; }

  void strokePath(Path const& path, GLVertex* vertex);

 private:
  float width_;
  float miter_limit_;
  float res_scale_;
  Paint::Cap cap_;
  Paint::Join join_;
};

}  // namespace skity

#endif

#endif  // SKITY_SRC_RENDER_GL_GL_PATH_STROKE