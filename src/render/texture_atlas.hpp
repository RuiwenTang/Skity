#ifndef SKITY_SRC_RENDER_TEXTURE_ATLAS_HPP
#define SKITY_SRC_RENDER_TEXTURE_ATLAS_HPP

#include <glm/glm.hpp>
#include <vector>

namespace skity {

/**
 * TextureAtlas is used to pack several small regions into a single
 * texture.
 *
 * The actual implementation is based on the article by Jukka Jylänki :
 * "A Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
 * Rectangle Bin Packing", February 27, 2010.
 */
class TextureAtlas {
 public:
  TextureAtlas(uint32_t width, uint32_t height, uint32_t depth);
  virtual ~TextureAtlas();

  /**
   * @brief               Allocate a new region in atlas.
   *
   * @param width         width of region to allocate
   * @param height        height of region to allocate
   * @return glm::ivec2   coordinates of the allocated region
   */
  glm::ivec4 AllocateRegion(uint32_t width, uint32_t height);

  /**
   * @brief         Upload data to the specified atlas region
   *
   * @param x       x coordinate of the region
   * @param y       y coordinate of the region
   * @param width   width of the region
   * @param height  height of the region
   * @param data    data to be uploaded into the specified region
   */
  void UploadRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                    uint8_t* data);

  /**
   * @brief Remove all allocated regions from the atlas.
   *
   */
  void Clear();

  /**
   * @brief
   *
   * @param new_width
   * @param new_height
   */
  void Resize(uint32_t new_width, uint32_t new_height);

  /**
   * @brief   Calculate texture uv coordinate
   *
   * @param x
   * @param y
   * @return glm::vec2
   */
  glm::vec2 CalculateUV(uint32_t x, uint32_t y);

  uint32_t Width() const { return width_; }
  uint32_t Height() const { return height_; }

 protected:
  virtual void OnUploadRegion(uint32_t x, uint32_t y, uint32_t width,
                              uint32_t height, uint8_t* data) {}

  virtual void OnResize(uint32_t new_width, uint32_t new_height) {}

 private:
  int32_t QueryFitY(int32_t index, uint32_t width, uint32_t height);
  void MergeNodes();

 private:
  // Width (in pixels) of the underlying texture
  uint32_t width_ = {};
  // Height(in pixels) of the underlying texture
  uint32_t height_ = {};
  uint32_t depth_ = {};
  // allocated surface size
  uint32_t used_ = {};
  // Atlas data
  uint8_t* data_ = nullptr;
  // Atlas has been modified
  bool modified_ = false;
  // Allocated nodes, [x, y, width]
  std::vector<glm::ivec3> nodes_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_TEXTURE_ATLAS_HPP