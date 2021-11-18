#include "src/render/hw/hw_path_raster.hpp"

#include "src/render/hw/hw_mesh.hpp"

namespace skity {

void HWPathRaster::AddStencilFront(uint32_t a, uint32_t b, uint32_t c) {
  stencil_front_buffer_.emplace_back(a);
  stencil_front_buffer_.emplace_back(b);
  stencil_front_buffer_.emplace_back(c);
}

void HWPathRaster::AddStencilBack(uint32_t a, uint32_t b, uint32_t c) {
  stencil_back_buffer_.emplace_back(a);
  stencil_back_buffer_.emplace_back(b);
  stencil_back_buffer_.emplace_back(c);
}

void HWPathRaster::AddColor(uint32_t a, uint32_t b, uint32_t c) {
  color_buffer_.emplace_back(a);
  color_buffer_.emplace_back(b);
  color_buffer_.emplace_back(c);
}

void HWPathRaster::AddAA(uint32_t a, uint32_t b, uint32_t c) {
  aa_buffer_.emplace_back(a);
  aa_buffer_.emplace_back(b);
  aa_buffer_.emplace_back(c);
}

uint32_t HWPathRaster::AppendVertex(float x, float y, float mix, float u,
                                    float v) {
  return (uint32_t)mesh_->AppendVertex(x, y, mix, u, v);
}

void HWPathRaster::OnBeginPath() {
  HWPathVisitor::OnBeginPath();

  stencil_front_buffer_.clear();
  stencil_front_count_ = stencil_front_start_ = 0;

  stencil_back_buffer_.clear();
  stencil_back_count_ = stencil_back_start_ = 0;

  color_buffer_.clear();
  color_count_ = color_start_ = 0;

  aa_buffer_.clear();
  aa_count_ = aa_start_ = 0;
}

void HWPathRaster::OnEndPath() {
  HWPathVisitor::OnEndPath();

  if (!stencil_front_buffer_.empty()) {
    stencil_front_start_ = mesh_->IndexBase();
    stencil_front_count_ = stencil_front_buffer_.size();
    mesh_->AppendIndices(stencil_front_buffer_);
  }

  if (!stencil_back_buffer_.empty()) {
    stencil_back_start_ = mesh_->IndexBase();
    stencil_back_count_ = stencil_back_buffer_.size();
    mesh_->AppendIndices(stencil_back_buffer_);
  }

  if (!color_buffer_.empty()) {
    color_start_ = mesh_->IndexBase();
    color_count_ = color_buffer_.size();
    mesh_->AppendIndices(color_buffer_);
  }

  if (!aa_buffer_.empty()) {
    aa_start_ = mesh_->IndexBase();
    aa_count_ = aa_buffer_.size();
    mesh_->AppendIndices(aa_buffer_);
  }
}

}  // namespace skity