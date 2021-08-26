#ifndef SKITY_SRC_RENDER_GL_GL_VERTEX_HPP
#define SKITY_SRC_RENDER_GL_GL_VERTEX_HPP

#include <array>
#include <vector>

namespace skity {

struct GLMeshRange {
  uint32_t front_start = 0;
  uint32_t front_count = 0;
  uint32_t back_start = 0;
  uint32_t back_count = 0;
  uint32_t aa_outline_start = 0;
  uint32_t aa_outline_count = 0;
};

class GLVertex {
  enum {
    GL_VERTEX_X = 0,
    GL_VERTEX_Y = 1,
    GL_VERTEX_ALPHA = 2,
    GL_VERTEX_TYPE = 3,
    GL_VERTEX_U = 4,
    GL_VERTEX_V = 5,
    GL_VERTEX_SIZE,
    GL_VERTEX_UV_OFFSET = GL_VERTEX_TYPE * sizeof(float),
    GL_VERTEX_STRIDE = GL_VERTEX_SIZE * sizeof(float),
  };

 public:
  enum {
    GL_VERTEX_TYPE_NORMAL = 0,
    GL_VERTEX_TYPE_QUAD = 1,
    GL_VERTEX_TYPE_QUAD_OFF = 2,
    GL_VERTEX_TYPE_RADIUS = 3,
    GL_VERTEX_TYPE_AA = 100,
  };

  GLVertex() = default;
  ~GLVertex() = default;

  using VertexData = std::array<float, GL_VERTEX_SIZE>;
  uint32_t AddPoint(float x, float y, uint32_t type, float u, float v);
  uint32_t AddPoint(float x, float y, float alpha, uint32_t type, float u,
                    float v);
  uint32_t AddPoint(VertexData const& data);
  void AddFront(uint32_t v1, uint32_t v2, uint32_t v3);
  void AddBack(uint32_t v1, uint32_t v2, uint32_t v3);
  void AddAAOutline(uint32_t v1, uint32_t v2, uint32_t v3);
  VertexData GetVertex(uint32_t index);
  void UpdateVertexData(VertexData const& data, uint32_t index);

  uint32_t FrontCount() const { return front_index.size(); }
  uint32_t BackCount() const { return back_index.size(); }
  uint32_t AAOutlineCount() const { return aa_index.size(); }

  void* GetVertexData() { return vertex_buffer.data(); }
  uint32_t GetVertexDataSize() { return vertex_buffer.size() * sizeof(float); }

  void* GetFrontIndexData() { return front_index.data(); }
  uint32_t GetFrontIndexDataSize() {
    return front_index.size() * sizeof(uint32_t);
  }

  void* GetAAIndexData() { return aa_index.data(); }
  uint32_t GetAAIndexDataSize() { return aa_index.size() * sizeof(uint32_t); }
  uint32_t CurrentIndex() { return vertex_buffer.size() / GL_VERTEX_SIZE; }

  void* GetBackIndexData() { return back_index.data(); }
  uint32_t GetBackIndexDataSize() {
    return back_index.size() * sizeof(uint32_t);
  }

  void Reset();

 private:
  std::vector<float> vertex_buffer;
  std::vector<uint32_t> front_index;
  std::vector<uint32_t> back_index;
  std::vector<uint32_t> aa_index;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_VERTEX_HPP