#include "src/render/gl/gl_mesh.hpp"

#include <cassert>

#include "src/render/gl/gl_interface.hpp"

namespace skity {

GLMesh::~GLMesh() {
  if (buffers_[0] != 0) {
    GL_CALL(DeleteBuffers, buffers_.size(), buffers_.data());
  }
  if (vao_ != 0) {
    GL_CALL(DeleteVertexArrays, 1, &vao_);
  }
}

void GLMesh::UploadVertexBuffer(void* data, uint32_t length) {
  assert(buffers_[0] != 0);

  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
  if (buffer_size_[0] < length) {
    buffer_size_[0] = length;
    GL_CALL(BufferData, GL_ARRAY_BUFFER, length, nullptr, GL_STATIC_DRAW);
  }

  GL_CALL(BufferSubData, GL_ARRAY_BUFFER, 0, length, data);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, 0);
}

void GLMesh::UploadQuadBuffer(void* data, uint32_t length) {
  assert(buffers_[4] != 0);

  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[4]);
  if (buffer_size_[4] < length) {
    buffer_size_[4] = length;
    GL_CALL(BufferData, GL_ARRAY_BUFFER, length, nullptr, GL_STATIC_DRAW);
  }

  GL_CALL(BufferSubData, GL_ARRAY_BUFFER, 0, length, data);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, 0);
}

void GLMesh::UploadFrontIndex(void* data, uint32_t length) {
  assert(buffers_[1] != 0);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
  if (buffer_size_[1] < length) {
    buffer_size_[1] = length;
    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, length, nullptr,
            GL_STATIC_DRAW);
  }
  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::UploadBackIndex(void* data, uint32_t length) {
  assert(buffers_[2] != 0);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
  if (buffer_size_[2] < length) {
    buffer_size_[2] = length;
    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, length, nullptr,
            GL_STATIC_DRAW);
  }
  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::UploadAAOutlineIndex(void* data, uint32_t length) {
  assert(buffers_[3] != 0);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[3]);
  if (buffer_size_[3] < length) {
    buffer_size_[3] = length;
    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, length, nullptr,
            GL_STATIC_DRAW);
  }
  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::UploadQuadIndex(void* data, uint32_t length) {
  assert(buffers_[5] != 0);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[5]);
  if (buffer_size_[5] < length) {
    buffer_size_[5] = length;
    GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, length, nullptr,
            GL_STATIC_DRAW);
  }
  GL_CALL(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::Init() {
  assert(vao_ == 0);
  assert(buffers_[0] == 0);
  assert(buffers_[1] == 0);
  assert(buffers_[2] == 0);
  assert(buffers_[3] == 0);
  assert(buffers_[4] == 0);
  assert(buffers_[5] == 0);

  GL_CALL(GenVertexArrays, 1, &vao_);
  GL_CALL(GenBuffers, buffers_.size(), buffers_.data());
}

void GLMesh::BindMesh() {
  GL_CALL(BindVertexArray, vao_);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
}

void GLMesh::BindNormalBuffer() {
  GL_CALL(BindVertexArray, vao_);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);

  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void*)0);
  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
          (void*)(2 * sizeof(float)));

  GL_CALL(DisableVertexAttribArray, 2);
  GL_CALL(DisableVertexAttribArray, 3);
}

void GLMesh::BindQuadBuffer() {
  GL_CALL(BindVertexArray, vao_);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[4]);

  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
          (void*)0);

  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
          (void*)(2 * sizeof(float)));

  GL_CALL(EnableVertexAttribArray, 2);
  GL_CALL(VertexAttribPointer, 2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
          (void*)(5 * sizeof(float)));

  GL_CALL(EnableVertexAttribArray, 3);
  GL_CALL(VertexAttribPointer, 3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
          (void*)(8 * sizeof(float)));
}

void GLMesh::BindFrontIndex() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
}

void GLMesh::BindBackIndex() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
}

void GLMesh::BindAAOutlineIndex() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[3]);
}

void GLMesh::BindQuadIndex() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[5]);
}

void GLMesh::UnBindMesh() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, 0);
  GL_CALL(BindVertexArray, 0);
}

GLMeshDraw::GLMeshDraw(uint32_t mode, uint32_t start, uint32_t count)
    : mode_(mode), start_(start), count_(count) {}

void GLMeshDraw::operator()() {
  //  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
          (void*)0);

  //  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
          (void*)(3 * sizeof(float)));

  GL_CALL(DrawElements, mode_, count_, GL_UNSIGNED_INT,
          (void*)(start_ * sizeof(GLuint)));
}

void GLMeshDraw2::operator()() const {
  GL_CALL(DrawElements, mode_, count_, GL_UNSIGNED_INT,
          (void*)(start_ * sizeof(GLuint)));
}

}  // namespace skity
