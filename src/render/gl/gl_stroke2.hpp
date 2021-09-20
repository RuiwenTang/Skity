
#ifndef SKITY_SRC_RENDER_GL_GL_STROKE2_HPP
#define SKITY_SRC_RENDER_GL_GL_STROKE2_HPP

#include <skity/geometry/point.hpp>

#include "src/render/gl/gl_path_visitor.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class GLStroke2 : public GLPathVisitor {
 public:
  GLStroke2(Paint const& paint, GLVertex2* gl_vertex);
  ~GLStroke2() override = default;

 protected:
  void HandleMoveTo(const Point& pt) override;
  void HandleLineTo(const Point& from, const Point& to) override;
  void HandleQuadTo(const Point& from, const Point& control,
                    const Point& end) override;
  void HandleClose() override;
  void HandleFinish() override;

 private:
  void HandleFirstAndEndCap();
  void HandleLineJoin(Vec2 const& from, Vec2 const& to);

  void HandleMiterJoinInternal(Vec2 const& center, Vec2 const& p1,
                               Vec2 const& d1, Vec2 const& p2, Vec2 const& d2);

  void HandleBevelJoinInternal(Vec2 const& center, Vec2 const& p1,
                               Vec2 const& p2, Vec2 const& curr_dir);

  void HandleRoundJoinInternal(Vec2 const& center, Vec2 const& p1,
                               Vec2 const& d1, Vec2 const& p2, Vec2 const& d2);

  void HandleSquareCapInternal(Vec2 const& pt, Vec2 const& dir);

  void HandleButtCapInternal(Vec2 const& pt, Vec2 const& dir);

  void HandleRoundCapInternal(Vec2 const& pt, Vec2 const& dir);

 private:
  float stroke_radius_;
  Lazy<Vec2> first_pt_{};
  Lazy<Vec2> first_dir_{};
  Lazy<Vec2> prev_dir_{};
  Lazy<Vec2> prev_pt_{};
  Lazy<Vec2> cur_pt_{};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_STROKE2_HPP
