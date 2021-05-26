#include "src/render/gl/gl_mesh.hpp"

// TODO use KHR header files
#include <glad/glad.h>

#include <cassert>

namespace skity {

GLMesh::~GLMesh() {
  if (buffers_[0] != 0) {
    glDeleteBuffers(3, buffers_.data());
  }
  if (vao_ != 0) {
    glDeleteVertexArrays(1, &vao_);
  }
}

void GLMesh::UploadVertexBuffer(void* data, uint32_t length) {
  assert(buffers_[0] != 0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
  glBufferData(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLMesh::UploadFrontIndex(void* data, uint32_t length) {
  assert(buffers_[1] != 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::UploadBackIndex(void* data, uint32_t length) {
  assert(buffers_[2] != 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::Init() {
  assert(vao_ == 0);
  assert(buffers_[0] == 0);
  assert(buffers_[1] == 0);
  assert(buffers_[2] == 0);

  glGenVertexArrays(1, &vao_);
  glGenBuffers(3, buffers_.data());
}

void GLMesh::BindMesh() {
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
}

void GLMesh::BindFrontIndex() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
}

void GLMesh::BindBackIndex() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
}

void GLMesh::UnBindMesh() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
}  // namespace skity