#ifndef SKITY_SRC_RENDER_HW_HW_MESH_HPP
#define SKITY_SRC_RENDER_HW_HW_MESH_HPP

#include <vector>

namespace skity {

class HWPipeline;

enum {
  HW_VERTEX_TYPE_LINE_NORMAL = 1,
  // circle fill, and u,v store circle center
  HW_VERTEX_TYPE_CIRCLE = 2,
  // quad fill, this used for path fill with quad, or maybe path stroke
  HW_VERTEX_TYPE_QUAD_IN = 3,
  HW_VERTEX_TYPE_QUAD_OUT = 4,
};

struct HWVertex {
  float x = 0.f;
  float y = 0.f;
  float mix = 0.f;
  float u = 0.f;
  float v = 0.f;

  HWVertex(float v1, float v2, float v3, float v4, float v5)
      : x(v1), y(v2), mix(v3), u(v4), v(v5) {}
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

  void UploadMesh(HWPipeline* pipeline);
  void ResetMesh();

 private:
  std::vector<HWVertex> raw_vertex_buffer_;
  std::vector<uint32_t> raw_index_buffer_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_MESH_HPP