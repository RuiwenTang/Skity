#include "src/render/gl/gl_vertex.hpp"

#include <cassert>

namespace skity {
uint32_t GLVertex::AddPoint(float x, float y, uint32_t type, float u, float v) {
  return AddPoint(x, y, 1.f, type, u, v);
}

uint32_t GLVertex::AddPoint(float x, float y, float alpha, uint32_t type,
                            float u, float v) {
  uint32_t index = vertex_buffer.size() / GL_VERTEX_SIZE;
  vertex_buffer.emplace_back(x);                      // x
  vertex_buffer.emplace_back(y);                      // y
  vertex_buffer.emplace_back(alpha * global_alpha_);  // alpha
  vertex_buffer.emplace_back(type);                   // type
  vertex_buffer.emplace_back(u);                      // u
  vertex_buffer.emplace_back(v);                      // v
  return index;
}

uint32_t GLVertex::AddPoint(VertexData const& data) {
  return AddPoint(data[0], data[1], data[2], data[3], data[4], data[6]);
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

void GLVertex::AddAAOutline(uint32_t v1, uint32_t v2, uint32_t v3) {
  aa_index.emplace_back(v1);
  aa_index.emplace_back(v2);
  aa_index.emplace_back(v3);
}

GLVertex::VertexData GLVertex::GetVertex(uint32_t index) {
  assert(index < vertex_buffer.size());
  VertexData result;

  result[GL_VERTEX_X] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_X];
  result[GL_VERTEX_Y] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_Y];
  result[GL_VERTEX_ALPHA] =
      vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_ALPHA];
  result[GL_VERTEX_TYPE] =
      vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_TYPE];
  result[GL_VERTEX_U] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_U];
  result[GL_VERTEX_V] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_V];

  return result;
}

void GLVertex::UpdateVertexData(VertexData const& data, uint32_t index) {
  assert(index < vertex_buffer.size());
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_X] = data[GL_VERTEX_X];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_Y] = data[GL_VERTEX_Y];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_ALPHA] =
      data[GL_VERTEX_ALPHA];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_TYPE] = data[GL_VERTEX_TYPE];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_U] = data[GL_VERTEX_U];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_V] = data[GL_VERTEX_V];
}

void GLVertex::Reset() {
  vertex_buffer.clear();
  front_index.clear();
  back_index.clear();
  aa_index.clear();
}

}  // namespace skity