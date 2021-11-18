#ifndef SKITY_SRC_RENDER_HW_HW_PATH_VISITOR_HPP
#define SKITY_SRC_RENDER_HW_HW_PATH_VISITOR_HPP

#include <memory>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <vector>

namespace skity {

class HWDrawRange;
class HWMesh;

class HWPathVisitor {
 public:
  HWPathVisitor(Paint const& paint);
  virtual ~HWPathVisitor() = default;

  void VisitPath(Path const& path, bool force_close);

 private:
  void HandleMoveTo(glm::vec2 const& p);
  void HandleLineTo(glm::vec2 const& p1, glm::vec2 const& p2);
  void HandleQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                    glm::vec2 const& p3);
  void HandleConicTo(glm::vec2 const& p1, glm::vec2 const& p2,
                     glm::vec2 const& p3, float weight);
  void HandleCubicTo(glm::vec2 const& p1, glm::vec2 const& p2,
                     glm::vec2 const& p3, glm::vec2 const& p4);
  void HandleClose();

 protected:
  glm::vec2 FirstPoint() const { return first_pt_; }

  glm::vec2 PrevDir() const { return prev_dir_; }

  virtual void OnBeginPath() = 0;

  virtual void OnEndPath() = 0;

  virtual void OnMoveTo(glm::vec2 const& p) = 0;

  virtual void OnLineTo(glm::vec2 const& p1, glm::vec2 const& p2) = 0;

  virtual void OnQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                        glm::vec2 const& p3) = 0;

  float StrokeWidth() const { return paint_.getStrokeWidth(); }
  float StrokeMiter() const { return paint_.getStrokeMiter(); }
  Paint::Cap LineCap() const { return paint_.getStrokeCap(); }
  Paint::Join LineJoin() const { return paint_.getStrokeJoin(); }

 private:
  Paint paint_;
  glm::vec2 first_pt_ = {};
  glm::vec2 prev_dir_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_PATH_VISITOR_HPP