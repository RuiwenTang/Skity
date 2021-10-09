#include "src/render/gl/gl_vertex.hpp"

#include <cassert>

namespace skity {

GLVertex::GLVertex() {
  vertex_buffer.reserve(128);
  front_index.reserve(128);
  back_index.reserve(128);
  aa_index.reserve(128);
}

uint32_t GLVertex::AddPoint(float x, float y, uint32_t type, float u, float v) {
  return AddPoint(x, y, 1.f, type, u, v);
}

uint32_t GLVertex::AddPoint(float x, float y, float alpha, uint32_t type,
                            float u, float v) {
  if (vertex_buffer.size() == vertex_buffer.capacity()) {
    vertex_buffer.reserve(2 * vertex_buffer.size());
  }
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
  if (front_index.size() == front_index.capacity()) {
    front_index.reserve(2 * front_index.size());
  }
  front_index.emplace_back(v1);
  front_index.emplace_back(v2);
  front_index.emplace_back(v3);
}

void GLVertex::AddBack(uint32_t v1, uint32_t v2, uint32_t v3) {
  if (back_index.size() == back_index.capacity()) {
    back_index.reserve(2 * back_index.size());
  }
  back_index.emplace_back(v1);
  back_index.emplace_back(v2);
  back_index.emplace_back(v3);
}

void GLVertex::AddAAOutline(uint32_t v1, uint32_t v2, uint32_t v3) {
  if (aa_index.size() == aa_index.capacity()) {
    aa_index.reserve(2 * aa_index.size());
  }
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

void GLVertex::Append(GLVertex* other, const Matrix& matrix) {
  uint32_t vertex_base = vertex_buffer.size() / GL_VERTEX_SIZE;
  uint32_t front_base = vertex_base;
  uint32_t back_base = vertex_base;
  uint32_t aa_base = vertex_base;

  for (size_t i = 0; i < other->vertex_buffer.size(); i += GL_VERTEX_SIZE) {
    Point p{other->vertex_buffer[i + GL_VERTEX_X],
            other->vertex_buffer[i + GL_VERTEX_Y], 0.f, 1.f};

    p = matrix * p;

    this->vertex_buffer.emplace_back(p.x);
    this->vertex_buffer.emplace_back(p.y);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_ALPHA]);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_TYPE]);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_U]);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_V]);
  }

  for (auto f : other->front_index) {
    this->front_index.emplace_back(f + front_base);
  }
  for (auto b : other->back_index) {
    this->back_index.emplace_back(b + back_base);
  }
  for (auto a : other->aa_index) {
    this->aa_index.emplace_back(a + aa_base);
  }
}

void GLVertex::Append(GLVertex* other, float scale, float tx, float ty) {
  uint32_t vertex_base = vertex_buffer.size() / GL_VERTEX_SIZE;
  uint32_t vertex_raw_base = vertex_buffer.size();
  uint32_t front_base = vertex_base;
  uint32_t back_base = vertex_base;
  uint32_t aa_base = vertex_base;

  uint32_t front_offset = this->front_index.size();
  uint32_t back_offset = this->back_index.size();
  uint32_t aa_offset = this->aa_index.size();
  this->vertex_buffer.resize(this->vertex_buffer.size() +
                             other->vertex_buffer.size());
  this->front_index.resize(this->FrontCount() + other->FrontCount());
  this->back_index.resize(this->BackCount() + other->BackCount());
  this->aa_index.resize(this->AAOutlineCount() + other->AAOutlineCount());

  const float* base_addr = other->vertex_buffer.data();
  float* this_addr = this->vertex_buffer.data();
  for (size_t i = 0; i < other->vertex_buffer.size(); i += GL_VERTEX_SIZE) {
    float x = base_addr[i + GL_VERTEX_X] * scale + tx;
    float y = base_addr[i + GL_VERTEX_Y] * scale + ty;

    this_addr[vertex_raw_base + i] = x;
    this_addr[vertex_raw_base + i + 1] = y;
    std::memcpy((this_addr + vertex_raw_base + i + 2), (base_addr + i + 2),
                3 * sizeof(float));
  }
  uint32_t* p = this->front_index.data();
  for (int32_t i = 0; i < other->front_index.size(); i++) {
    this->front_index[front_offset + i] = other->front_index[i] + front_base;
  }
  p = this->back_index.data();
  for (int32_t i = 0; i < other->back_index.size(); i++) {
    this->back_index[back_offset + i] = other->back_index[i] + back_base;
  }
  p = this->aa_index.data();
  for (int32_t i = 0; i < other->aa_index.size(); i++) {
    this->aa_index[aa_offset + i] = other->aa_index[i] + aa_base;
  }
}

GLVertex2::Data::Data(float x, float y, float mix, float u, float v)
    : x(x), y(y), mix(mix), u(u), v(v) {}

uint32_t GLVertex2::AddPoint(float x, float y, float mix, float u, float v) {
  uint32_t i = vertex_buffer.size();
  vertex_buffer.emplace_back(x, y, mix, u, v);
  return i;
}

void GLVertex2::AddFront(uint32_t a, uint32_t b, uint32_t c) {
  front_index.emplace_back(a);
  front_index.emplace_back(b);
  front_index.emplace_back(c);
}

void GLVertex2::AddBack(uint32_t a, uint32_t b, uint32_t c) {
  back_index.emplace_back(a);
  back_index.emplace_back(b);
  back_index.emplace_back(c);
}

void GLVertex2::AddAA(uint32_t a, uint32_t b, uint32_t c) {
  aa_index.emplace_back(a);
  aa_index.emplace_back(b);
  aa_index.emplace_back(c);
}

void GLVertex2::AddQuad(uint32_t a, uint32_t b, uint32_t c) {
  quad_index.emplace_back(a);
  quad_index.emplace_back(b);
  quad_index.emplace_back(c);
}

std::pair<void*, size_t> GLVertex2::GetVertexDataSize() {
  return std::make_pair(static_cast<void*>(vertex_buffer.data()),
                        vertex_buffer.size() * sizeof(GLVertex2::Data));
}

std::pair<void*, size_t> GLVertex2::GetFrontDataSize() {
  return std::make_pair(static_cast<void*>(front_index.data()),
                        front_index.size() * sizeof(uint32_t));
}

std::pair<void*, size_t> GLVertex2::GetBackDataSize() {
  return std::make_pair(static_cast<void*>(back_index.data()),
                        back_index.size() * sizeof(uint32_t));
}

std::pair<void*, size_t> GLVertex2::GetAADataSize() {
  return std::make_pair(static_cast<void*>(aa_index.data()),
                        aa_index.size() * sizeof(uint32_t));
}

std::pair<void*, size_t> GLVertex2::GetQuadDataSize() {
  return std::make_pair(static_cast<void*>(quad_index.data()),
                        quad_index.size() * sizeof(uint32_t));
}

GLVertex2::Data GLVertex2::GetVertexData(uint32_t index) const {
  return vertex_buffer[index];
}

void GLVertex2::UpdateVertexData(uint32_t index, const GLVertex2::Data& data) {
  vertex_buffer[index] = data;
}

void GLVertex2::Reset() {
  vertex_buffer.clear();
  front_index.clear();
  back_index.clear();
  aa_index.clear();
  quad_index.clear();
}

GLQuadRange::GLQuadRange(uint32_t quadStart, uint32_t quadCount,
                         const Vec2& start, const Vec2& control,
                         const Vec2& anEnd, float offset)
    : quad_start(quadStart),
      quad_count(quadCount),
      start(start),
      control(control),
      end(anEnd),
      offset(offset) {}

}  // namespace skity
