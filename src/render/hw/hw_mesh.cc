#include "src/render/hw/hw_mesh.hpp"

namespace skity {

size_t HWMesh::AppendVertex(float x, float y, float mix, float u, float v) {
  size_t base = VertexBase();

  raw_vertex_buffer_.emplace_back(x, y, mix, u, v);

  return base;
}

size_t HWMesh::AppendVertex(const HWVertex &vertex) {
  size_t base = VertexBase();

  raw_vertex_buffer_.emplace_back(vertex);

  return base;
}

size_t HWMesh::AppendIndices(const std::vector<uint32_t> &indices) {
  size_t base = IndexBase();

  raw_index_buffer_.insert(raw_index_buffer_.end(), indices.begin(),
                           indices.end());

  return base;
}

void HWMesh::ResetMesh() {
  raw_index_buffer_.clear();
  raw_vertex_buffer_.clear();
}

}  // namespace skity