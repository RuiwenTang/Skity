
#ifndef SKITY_RENDER_GL_GL_PATH_VISITOR_HPP
#define SKITY_RENDER_GL_GL_PATH_VISITOR_HPP

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class GLVertex2;
class GLMeshRange;

class GLPathVisitor {
 public:
  explicit GLPathVisitor(Paint const& paint, GLVertex2* gl_vertex);
  virtual ~GLPathVisitor() = default;

  GLMeshRange VisitPath(Path const& path, bool force_close);

 protected:
  GLVertex2* GetGLVertex() const { return gl_vertex_; }
  bool IsAntiAlias() const { return anti_alias_; }
  Paint::Style GetStyle() const { return style_; }
  Paint::Join GetJoin() const { return join_; }
  Paint::Cap GetCap() const { return cap_; }
  float GetStrokeWidth() const { return stroke_width_; }
  float GetMiterLimit() const { return miter_limit_; }

  virtual void HandleMoveTo(Point const& pt) = 0;
  virtual void HandleLineTo(Point const& from, Point const& to) = 0;
  virtual void HandleQuadTo(Point const& from, Point const& control,
                            Point const& end) = 0;
  virtual void HandleClose() = 0;
  virtual void HandleFinish(GLMeshRange* range) = 0;

 private:
  void HandleConicToInternal(Point const& start, Point const& control,
                             Point const& end, float weight);

  void HandleCubicToInternal(Point const& start, Point const& control1,
                             Point const& control2, Point const& end);

 private:
  GLVertex2* gl_vertex_;
  bool anti_alias_;
  Paint::Style style_;
  Paint::Join join_;
  Paint::Cap cap_;
  float stroke_width_;
  float miter_limit_;
};

}  // namespace skity

#endif  // SKITY_RENDER_GL_GL_PATH_VISITOR_HPP
