
#ifndef SKITY_SRC_RENDER_GL_GL_FILL2_HPP
#define SKITY_SRC_RENDER_GL_GL_FILL2_HPP

#include "src/render/gl/gl_path_visitor.hpp"

namespace skity {

class GLFill2 : public GLPathVisitor {
 public:
  GLFill2(Paint const& paint, GLVertex2* gl_vertex);

  ~GLFill2() override = default;

 protected:
  void HandleMoveTo(const Point& pt) override;
  void HandleLineTo(const Point& from, const Point& to) override;
  void HandleQuadTo(const Point& from, const Point& control,
                    const Point& end) override;
  void HandleClose() override;
  void HandleFinish(GLMeshRange* range) override;

 private:
  Vec2 first_pt_ = {};
  Vec2 first_pt_dir_ = {};
  Vec2 prev_to_dir_ = {};
  int32_t first_pt_index_ = -1;
  int32_t prev_pt_index_ = -1;
  int32_t prev_pt_aa_index_ = -1;
  float anti_alias_width_ = 10.f;
  bool first_is_line_ = false;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_FILL2_HPP
