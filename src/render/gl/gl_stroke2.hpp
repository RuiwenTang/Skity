
#ifndef SKITY_SRC_RENDER_GL_GL_STROKE2_HPP
#define SKITY_SRC_RENDER_GL_GL_STROKE2_HPP

#include <skity/geometry/point.hpp>
#include <utility>

#include "src/render/gl/gl_path_visitor.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class GLStroke2 : public GLPathVisitor {
  struct PathCMD {
    Path::Verb verb;
    Vec2 p1;
    Vec2 p2;
    Vec2 p3;
    Vec2 p4;
  };

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

  void HandleSquareCapInternal(Vec2 const& pt, Vec2 const& dir);

  void HandleButtCapInternal(Vec2 const& pt, Vec2 const& dir);

  void HandleRoundCapInternal(Vec2 const& pt, Vec2 const& dir);

  Vec2 GetPrevStartDirection();
  Vec2 GetPrevEndDirection();
  Vec2 GetPrevStartPoint();
  Vec2 GetPrevEndPoint();

  void HandlePrevPathCMD(PathCMD const& curr_path_cmd);
  void HandlePrevPathCMDEnd();
  std::pair<Vec2, Vec2> CalculateLineJoinPoints(PathCMD const& curr_path_cmd);

  void HandleLineToInternal(PathCMD const& curr_path_cmd);

  void GenerateLineSquare(Vec2 const& p1, Vec2 const& p2, Vec2 const& p3,
                          Vec2 const& p4);

 private:
  float stroke_radius_;
  Lazy<Vec2> first_pt_{};
  Lazy<Vec2> first_dir_{};
  Lazy<Vec2> prev_join_1_{};
  Lazy<Vec2> prev_join_2_{};
  Lazy<PathCMD> prev_cmd_{};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_STROKE2_HPP
