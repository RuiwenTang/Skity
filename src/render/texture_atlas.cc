#include "src/render/texture_atlas.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>

namespace skity {

TextureAtlas::TextureAtlas(uint32_t width, uint32_t height, uint32_t depth)
    : width_(width), height_(height), depth_(depth) {
  // one pixel border for sampling texture
  glm::ivec3 node = {1, 1, width - 2};

  nodes_.emplace_back(node);
  modified_ = true;
  data_ = (uint8_t *)std::malloc(width * height * depth * sizeof(uint8_t));
  std::memset(data_, 0, width * height * depth * sizeof(uint8_t));
}

TextureAtlas::~TextureAtlas() {
  if (data_) {
    std::free(data_);
  }
}

glm::ivec4 TextureAtlas::AllocateRegion(uint32_t width, uint32_t height) {
  glm::ivec4 region = {0, 0, (int32_t)width, (int32_t)height};

  int32_t best_index = -1;
  int32_t best_height = std::numeric_limits<int32_t>::max();
  int32_t best_width = std::numeric_limits<int32_t>::max();

  int32_t y = -1;
  for (int32_t i = 0; i < nodes_.size(); i++) {
    y = QueryFitY(i, width, height);
    if (y > 0) {
      glm::ivec3 node = nodes_[i];
      if ((y + height) < best_height || (((y + height) == best_height) &&
                                         (node.z > 0 && node.z < best_width))) {
        best_index = i;
        best_height = y + height;
        best_width = node.z;
        region.x = node.x;
        region.y = y;
      }
    }
  }

  if (best_index == -1) {
    region.x = -1;
    region.y = -1;
    region.z = 0;
    region.w = 0;
    return region;
  }

  glm::ivec3 node = {region.x, region.y + height, width};

  nodes_.insert(nodes_.begin() + best_index, node);

  for (auto i = nodes_.begin() + best_index + 1; i != nodes_.end(); i++) {
    auto i_node = i;
    auto p_node = i - 1;

    if (i_node->x < (p_node->x + p_node->z)) {
      int32_t shrink = p_node->x + p_node->z - i_node->x;
      i_node->x += shrink;
      i_node->z -= shrink;

      if (i_node->z <= 0) {
        i = nodes_.erase(i_node);
        i--;
      } else {
        break;
      }
    } else {
      break;
    }
  }
  MergeNodes();
  used_ += width * height;
  modified_ = true;
  return region;
}

void TextureAtlas::UploadRegion(uint32_t x, uint32_t y, uint32_t width,
                                uint32_t height, size_t row_bytes,uint8_t *data) {
  std::memcpy(data_ + (y * this->width_ + x) * depth_, data,
              width * height * depth_);

  this->modified_ = true;
  this->OnUploadRegion(x, y, width, height, row_bytes,data);
}

int32_t TextureAtlas::QueryFitY(int32_t index, uint32_t width,
                                uint32_t height) {
  glm::ivec3 node = nodes_[index];

  int32_t x = node.x;
  int32_t y = node.y;

  int32_t width_left = width;
  int32_t i = index;

  if (x + width > this->width_ - 1) {
    return -1;
  }

  y = node.y;
  while (width_left > 0) {
    node = nodes_[i];
    if (node.y > y) {
      y = node.y;
    }
    if (y + height > this->height_ - 1) {
      return -1;
    }

    width_left -= node.z;
    i++;
  }

  return y;
}

void TextureAtlas::Clear() {
  nodes_.clear();
  nodes_.emplace_back(glm::ivec3{1, 1, width_ - 2});
  used_ = 0;
  std::memset(data_, 0, width_ * height_ * depth_);
}

void TextureAtlas::Resize(uint32_t new_width, uint32_t new_height) {
  assert(new_width >= width_);
  assert(new_height >= height_);

  uint32_t old_width = width_;
  uint32_t old_height = height_;
  uint8_t *old_data = data_;

  this->data_ = (uint8_t *)std::malloc(new_width * new_height * depth_);

  this->width_ = new_width;
  this->height_ = new_height;

  if (new_width > old_width) {
    glm::ivec3 node = {old_width - 1, 1, new_width - old_width};
    nodes_.emplace_back(node);
  }

  size_t pixel_size = depth_;
  size_t old_row_size = old_width * pixel_size;

  this->OnResize(new_width, new_height);

  UploadRegion(1, 1, old_width - 2, old_height - 2,old_row_size,
               old_data + old_row_size + pixel_size);

  std::free(old_data);
}

glm::vec2 TextureAtlas::CalculateUV(uint32_t x, uint32_t y) {
  float u = (float)x / (float)width_;
  float v = (float)y / (float)height_;

  return {u, v};
}

void TextureAtlas::MergeNodes() {
  for (auto node = nodes_.begin(); node < nodes_.end() - 1; node++) {
    auto next = node + 1;
    if (node->y == next->y) {
      node->z += next->z;
      nodes_.erase(next);
    }
  }
}

}  // namespace skity
