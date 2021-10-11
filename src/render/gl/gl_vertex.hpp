#ifndef SKITY_SRC_RENDER_GL_GL_VERTEX_HPP
#define SKITY_SRC_RENDER_GL_GL_VERTEX_HPP

#include <array>
#include <skity/geometry/point.hpp>
#include <utility>
#include <vector>

namespace skity {

struct GLQuadRange {
  uint32_t quad_start = 0;
  uint32_t quad_count = 0;
  Vec2 start;
  Vec2 control;
  Vec2 end;
  float offset;

  GLQuadRange(uint32_t quadStart, uint32_t quadCount, const Vec2& start,
              const Vec2& control, const Vec2& anEnd, float offset);
};

struct GLMeshRange {
  uint32_t front_start = 0;
  uint32_t front_count = 0;
  uint32_t back_start = 0;
  uint32_t back_count = 0;
  uint32_t aa_outline_start = 0;
  uint32_t aa_outline_count = 0;
  std::vector<GLQuadRange> quad_front_range = {};
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

  GLVertex();
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

  void UpdateGlobalAlpha(float alpha) { global_alpha_ = alpha; }

  void Reset();

  void Append(GLVertex* other, Matrix const& matrix);
  void Append(GLVertex* other, float scale, float tx, float ty);

 private:
  std::vector<float> vertex_buffer;
  std::vector<uint32_t> front_index;
  std::vector<uint32_t> back_index;
  std::vector<uint32_t> aa_index;
  float global_alpha_ = 1.0f;
};

class GLVertex2 {
 public:
  struct Data {
    float x;
    float y;
    float mix;
    float u;
    float v;

    Data(float x, float y, float mix, float u, float v);
  };

  struct QuadData {
    float x;
    float y;
    float mix;
    float u;
    float v;
    float offset;
    float p1x;
    float p1y;
    float p2x;
    float p2y;
    float p3x;
    float p3y;
  };

  GLVertex2() = default;
  ~GLVertex2() = default;

  enum VertexType {
    STROKE_AA = 100,
    // basic type
    NONE = 0,
    LINE_EDGE = 1,
    LINE_CAP = 2,
    LINE_BEVEL_JOIN = 3,
    LINE_ROUND_JOIN = 4,
    FILL_QUAD_IN = 5,
    QUAD_OUT = 6,
    LINE_ROUND = 7,
    STROKE_QUAD = 8,
    FILL_EDGE = 9,
  };

  uint32_t AddPoint(float x, float y, float mix, float u, float v);
  void AddFront(uint32_t a, uint32_t b, uint32_t c);
  void AddBack(uint32_t a, uint32_t b, uint32_t c);
  void AddAA(uint32_t a, uint32_t b, uint32_t c);
  void AddQuad(uint32_t a, uint32_t b, uint32_t c);

  uint32_t FrontCount() const { return front_index.size(); }
  uint32_t BackCount() const { return back_index.size(); }
  uint32_t AACount() const { return aa_index.size(); }
  uint32_t QuadCount() const { return quad_index.size(); }

  std::pair<void*, size_t> GetVertexDataSize();
  std::pair<void*, size_t> GetFrontDataSize();
  std::pair<void*, size_t> GetBackDataSize();
  std::pair<void*, size_t> GetAADataSize();
  std::pair<void*, size_t> GetQuadDataSize();

  GLVertex2::Data GetVertexData(uint32_t index) const;

  void UpdateVertexData(uint32_t index, GLVertex2::Data const& data);

  void Reset();

 private:
  std::vector<Data> vertex_buffer;
  std::vector<QuadData> quad_buffer;
  std::vector<uint32_t> front_index;
  std::vector<uint32_t> back_index;
  std::vector<uint32_t> aa_index;
  std::vector<uint32_t> quad_index;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_VERTEX_HPP
