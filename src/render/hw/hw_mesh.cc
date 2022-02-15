#include "src/render/hw/hw_mesh.hpp"

namespace skity {

size_t HWMesh::AppendIndices(const std::vector<uint32_t> &indices) {
  size_t base = IndexBase();

  raw_index_buffer_.insert(raw_index_buffer_.end(), indices.begin(),
                           indices.end());

  return base;
}

void HWMesh::UploadMesh(HWRenderer *renderer) {
  renderer->UploadIndexBuffer(raw_index_buffer_.data(),
                              sizeof(uint32_t) * raw_index_buffer_.size());

  OnUploadMesh(renderer);
}

void HWMesh::ResetMesh() {
  raw_index_buffer_.clear();

  OnResetMesh();
}

}  // namespace skity