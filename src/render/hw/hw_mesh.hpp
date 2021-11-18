#ifndef SKITY_SRC_RENDER_HW_HW_MESH_HPP
#define SKITY_SRC_RENDER_HW_HW_MESH_HPP

#include <vector>

namespace skity {

enum {
  // this is used for stroke fill path outline in fill path without MSAA
  HW_VERTEX_TYPE_AA_OUTLINE = 0,
  // normal triangle fill no more calculation
  HW_VERTEX_TYPE_LINE_NORMAL = 1,
  // line with aa alpha calculation, this is used in onDrawLine, or onDrawRect,
  // which path only contains line
  HW_VERTEX_TYPE_LINE_AA = 2,
  // circle fill, and u,v store
  HW_VERTEX_TYPE_CIRCLE = 3,
  // circle fill, with aa alpha calculation
  HW_VERTEX_TYPE_CIRCLE_AA = 4,
  // quad fill, this used for path fill with quad, or maybe path stroke
  HW_VERTEX_TYPE_QUAD_IN = 5,
  HW_VERTEX_TYPE_QUAD_OUT = 6,
};

struct HWVertex {
  float x = 0.f;
  float y = 0.f;
  float mix = 0.f;
  float u = 0.f;
  float v = 0.f;
};

enum class HWDrawType {
  STENCIL_FRONT,
  STENCIL_BACK,
  COLOR,
  AA,
};

struct HWDrawRange {
  uint32_t start = 0;
  uint32_t count = 0;
  HWDrawType type = HWDrawType::COLOR;
};

class HWMesh {
 public:
  HWMesh() = default;
  ~HWMesh() = default;

  size_t AppendVertex(float x, float y, float mix, float u = 0.f,
                      float v = 0.f);
  size_t AppendVertex(HWVertex const& vertex);

  size_t VertexBase() { return raw_vertex_buffer_.size(); }

  size_t IndexBase() { return raw_index_buffer_.size(); }

  size_t AppendIndices(std::vector<uint32_t> const& indices);

  void Reset();

 private:
  std::vector<HWVertex> raw_vertex_buffer_;
  std::vector<uint32_t> raw_index_buffer_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_MESH_HPP