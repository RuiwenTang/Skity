#ifndef SKITY_SRC_RENDER_SW_SW_PATH_STROKE_HPP
#define SKITY_SRC_RENDER_SW_SW_PATH_STROKE_HPP

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class SWPathStroke final {
 public:
  SWPathStroke() = default;
  ~SWPathStroke() = default;

  void StrokePath(Path const& src, Paint const& paint);

 private:
  float StrokeWidth() const;

  void HandleMoveTo(glm::vec2 const& pt);

  void HandleLineTo(glm::vec2 const& p1, glm::vec2 const& p2);

  void HandleQuadTo(glm::vec2 const& p1, glm::vec2 const& control,
                    glm::vec2 const& p2);

  void HandleCubicTo(glm::vec2 const& p1, glm::vec2 const& control1,
                     glm::vec2 const& control2, glm::vec2 const& p2);

  void HandleConicTo(glm::vec2 const& p1, glm::vec2 const& control,
                     glm::vec2 const& p2, float weight);

  void HandleLineJoin(glm::vec2 const& p1, glm::vec2 const& p2);

  void HandleInnerJoin(Path* path, Vec2 const& center, Vec2 const& p1,
                       Vec2 const& p2);

  void HandleMiterJoinInternal(Path* path, Vec2 const& center, Vec2 const& p1,
                               Vec2 const& d1, Vec2 const& p2, Vec2 const& d2);

  void HandleBevelJoinInternal(Path* path, Vec2 const& center, Vec2 const& p1,
                               Vec2 const& p2);

  void HandleRoundJoinInternal(Path* path, Vec2 const& center, Vec2 const& p1,
                               Vec2 const& d1, Vec2 const& p2, Vec2 const& d2);

  void HandleLineCap(Path* path, Vec2 const& center, Vec2 const& p1,
                     Vec2 const& p2, Vec2 const& out_dir);

  void HandleRoundCap(Path* path, Vec2 const& center, Vec2 const& p1,
                      Vec2 const& p2, Vec2 const& out_dir);

  void HandleButtCap(Path* path, Vec2 const& p1, Vec2 const& p2);

  void HandleSquareCap(Path* path, Vec2 const& p1, Vec2 const& p2,
                       Vec2 const& out_dir);
  void HandleEnd();

 private:
  Path outer_ = {};
  Path inner_ = {};
  Paint::Cap cap_ = Paint::Cap::kButt_Cap;
  Paint::Join join_ = Paint::Join::kBevel_Join;
  float stroke_width_ = 1.f;
  float stroke_miter_limit_ = 5.f;
  glm::vec2 first_pt_ = {};
  glm::vec2 first_dir_ = {};
  glm::vec2 first_p1_ = {};
  glm::vec2 first_p2_ = {};
  glm::vec2 prev_pt_ = {};
  glm::vec2 prev_dir_ = {};
  glm::vec2 curr_pt_ = {};
  glm::vec2 curr_pt1_ = {};
  glm::vec2 curr_pt2_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_SW_SW_PATH_STROKE_HPP