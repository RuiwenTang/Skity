#ifndef SKITY_SRC_RENDER_HW_HW_MESH_HPP
#define SKITY_SRC_RENDER_HW_HW_MESH_HPP

#include <vector>

#include "src/render/hw/hw_renderer.hpp"

namespace skity {

enum {
  HW_VERTEX_TYPE_LINE_NORMAL = 1,
  // circle fill, and u,v store circle center
  HW_VERTEX_TYPE_CIRCLE = 2,
  // quad fill, this used for path fill with quad, or maybe path stroke
  HW_VERTEX_TYPE_QUAD_IN = 3,
  HW_VERTEX_TYPE_QUAD_OUT = 4,
  // text fill,
  HW_VERTEX_TYPE_TEXT = 5,
  // geometry quad stroke
  HW_VERTEX_TYPE_QUAD_STROKE = 6,
};

class HWMesh {
 public:
  virtual ~HWMesh() = default;

  virtual size_t AppendVertex(float x, float y, float mix, float u = 0.f,
                              float v = 0.f) = 0;

  virtual size_t VertexBase() = 0;

  size_t IndexBase() { return raw_index_buffer_.size(); }

  size_t AppendIndices(std::vector<uint32_t> const& indices);

  void UploadMesh(HWRenderer* renderer);

  void ResetMesh();

 protected:
  virtual void OnUploadMesh(HWRenderer* renderer) = 0;

  virtual void OnResetMesh() = 0;

 private:
  std::vector<uint32_t> raw_index_buffer_;
};

template <class T>
class HWMeshImpl : public HWMesh {
 public:
  using VertexT = T;

  HWMeshImpl() = default;
  ~HWMeshImpl() override = default;

  size_t AppendVertex(float x, float y, float mix, float u, float v) override {
    size_t base = VertexBase();

    raw_vertex_buffer_.emplace_back(x, y, mix, u, v);

    return base;
  }

  size_t VertexBase() override { return raw_vertex_buffer_.size(); }

 protected:
  void OnUploadMesh(HWRenderer* renderer) override {
    renderer->UploadVertexBuffer(raw_vertex_buffer_.data(),
                                 sizeof(VertexT) * raw_vertex_buffer_.size());
  }

  void OnResetMesh() override { raw_vertex_buffer_.clear(); }

 private:
  std::vector<T> raw_vertex_buffer_ = {};
};

struct HWVertexNormal {
  float x = 0.f;
  float y = 0.f;
  float mix = 0.f;
  float u = 0.f;
  float v = 0.f;

  HWVertexNormal(float v1, float v2, float v3, float v4, float v5)
      : x(v1), y(v2), mix(v3), u(v4), v(v5) {}
};

struct HWVertexGS {
  float x = 0.f;
  float y = 0.f;
  float mix = 0.f;

  HWVertexGS(float v1, float v2, float v3, float, float)
      : x(v1), y(v2), mix(v3) {}
};

using HWMeshNormal = HWMeshImpl<HWVertexNormal>;

using HWMeshGS = HWMeshImpl<HWVertexGS>;

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_MESH_HPP