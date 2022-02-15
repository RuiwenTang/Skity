#ifndef SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP

#include <vector>

#include "src/render/hw/hw_path_visitor.hpp"

namespace skity {

class HWMesh;

class HWPathRaster : public HWPathVisitor {
 public:
  HWPathRaster(HWMesh* mesh, Paint const& paint, bool use_gs)
      : HWPathVisitor(mesh, paint, use_gs) {}
  ~HWPathRaster() override = default;

  void FillPath(Path const& path);
  void StrokePath(Path const& path);

 protected:
  void OnBeginPath() override;
  void OnEndPath() override;
  void OnMoveTo(glm::vec2 const& p) override;

  void OnLineTo(glm::vec2 const& p1, glm::vec2 const& p2) override;

  void OnQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                glm::vec2 const& p3) override;

 private:
  void StrokeLineTo(glm::vec2 const& p1, glm::vec2 const& p2);
  void StrokeQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                    glm::vec2 const& p3);
  void FillLineTo(glm::vec2 const& p1, glm::vec2 const& p2);
  void FillQuadTo(glm::vec2 const& p1, glm::vec2 const& p2,
                  glm::vec2 const& p3);
  void HandleLineJoin(glm::vec2 const& p1, glm::vec2 const& p2,
                      float stroke_radius);

  void HandleMiterJoinInternal(Vec2 const& center, Vec2 const& p1,
                               Vec2 const& d1, Vec2 const& p2, Vec2 const& d2);

  void HandleBevelJoinInternal(Vec2 const& center, Vec2 const& p1,
                               Vec2 const& p2, Vec2 const& curr_dir);

  void HandleRoundJoinInternal(Vec2 const& center, Vec2 const& p1,
                               Vec2 const& d1, Vec2 const& p2, Vec2 const& d2);

  void AppendQuadOrSplitRecursively(std::array<Vec2, 3> const& outer,
                                    std::array<Vec2, 3> const& inner);

 private:
  bool stroke_ = false;
  glm::vec2 first_pt_ = {};
  int32_t first_pt_index_ = -1;
  glm::vec2 first_pt_dir_ = {};
  glm::vec2 prev_pt_ = {};
  glm::vec2 curr_pt_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP