#ifndef SKITY_SRC_RENDER_GL_GL_MESH_HPP
#define SKITY_SRC_RENDER_GL_GL_MESH_HPP

#include <array>
#include <cstdint>

namespace skity {

/**
 * @class GLMesh
 *  this class contains:
 *    1. vao
 *    2. raw_vertex_buffer
 *    3. index_buffer for front triangle list
 *    4. index_buffer for back triangle list
 *    5. index_buffer for AA triangle list
 */
class GLMesh {
 public:
  GLMesh() = default;
  ~GLMesh();

  void UploadVertexBuffer(void* data, uint32_t length);
  void UploadFrontIndex(void* data, uint32_t length);
  void UploadBackIndex(void* data, uint32_t length);
  void uploadAaOutlineIndex(void* data, uint32_t length);
  // generate vao and gl_buffers
  void Init();

  void BindMesh();

  void BindFrontIndex();

  void BindBackIndex();

  void BindAAOutlineIndex();

  void UnBindMesh();

 private:
  uint32_t vao_ = 0;
  // [vertex_buffer, front_index, back_index, aa_index]
  std::array<uint32_t, 4> buffers_ = {};
  std::array<uint64_t, 4> buffer_size_{};
};

/**
 * @brief Helper class to draw fixed layout vertex buffer
 *  [x, y, alpha, type , curve, v]
 */
class GLMeshDraw final {
 public:
  GLMeshDraw(uint32_t mode, uint32_t start, uint32_t count);
  ~GLMeshDraw() = default;
  void operator()();

 private:
  uint32_t mode_;
  uint32_t start_;
  uint32_t count_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_MESH_HPP
