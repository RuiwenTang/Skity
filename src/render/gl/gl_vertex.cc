#include "src/render/gl/gl_vertex.hpp"

#include <cassert>

namespace skity {
uint32_t GLVertex::AddPoint(float x, float y, uint32_t type, float u, float v) {
  uint32_t index = vertex_buffer.size() / GL_VERTEX_SIZE;
  vertex_buffer.emplace_back(x);     // x
  vertex_buffer.emplace_back(y);     // y
  vertex_buffer.emplace_back(type);  // type
  vertex_buffer.emplace_back(u);     // u
  vertex_buffer.emplace_back(v);     // v
  return index;
}

uint32_t GLVertex::AddPoint(VertexData const& data) {
  return AddPoint(data[0], data[1], data[2], data[3], data[4]);
}

void GLVertex::AddFront(uint32_t v1, uint32_t v2, uint32_t v3) {
  front_index.emplace_back(v1);
  front_index.emplace_back(v2);
  front_index.emplace_back(v3);
}

void GLVertex::AddBack(uint32_t v1, uint32_t v2, uint32_t v3) {
  back_index.emplace_back(v1);
  back_index.emplace_back(v2);
  back_index.emplace_back(v3);
}

GLVertex::VertexData GLVertex::GetVertex(uint32_t index) {
  assert(index < vertex_buffer.size());
  VertexData result;

  result[GL_VERTEX_X] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_X];
  result[GL_VERTEX_Y] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_Y];
  result[GL_VERTEX_TYPE] =
      vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_TYPE];
  result[GL_VERTEX_U] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_U];
  result[GL_VERTEX_V] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_V];

  return result;
}

}  // namespace skity