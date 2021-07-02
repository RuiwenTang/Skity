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
 */
class GLMesh {
 public:
  GLMesh() = default;
  ~GLMesh();

  void UploadVertexBuffer(void* data, uint32_t length);
  void UploadFrontIndex(void* data, uint32_t length);
  void UploadBackIndex(void* data, uint32_t length);

  // generate vao and gl_buffers
  void Init();

  void BindMesh();

  void BindFrontIndex();

  void BindBackIndex();

  void UnBindMesh();

 private:
  uint32_t vao_ = 0;
  // [vertex_buffer, front_index, back_index]
  std::array<uint32_t, 3> buffers_ = {};
};
}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_MESH_HPP