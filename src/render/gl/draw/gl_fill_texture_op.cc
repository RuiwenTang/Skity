#include "src/render/gl/draw/gl_fill_texture_op.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_texture.hpp"
#include "src/render/gl/gl_interface.hpp"

namespace skity {

GLFillTextureOp::GLFillTextureOp(uint32_t front_start, uint32_t front_count,
                                 uint32_t back_start, uint32_t back_count,
                                 uint32_t aa_start, uint32_t aa_count,
                                 GLTextureShader* shader, GLMesh* mesh)
    : GLDrawMeshOpAA(front_start, front_count, back_start, back_count, aa_start,
                     aa_count, shader, mesh),
      shader_(shader),
      local_matrix_(glm::identity<Matrix>()) {}

void GLFillTextureOp::SetLocalMatrix(Matrix const& matrix) {
  local_matrix_ = matrix;
}

void GLFillTextureOp::SetBounds(Point const& p1, Point const& p2) {
  bounds_[0] = p1;
  bounds_[1] = p2;
}

void GLFillTextureOp::SetTexture(const GLTexture* texture) {
  texture_ = texture;
}

void GLFillTextureOp::OnBeforeDraw(bool has_clip) {
  GLDrawMeshOpAA::OnBeforeDraw(has_clip);
  // matrixs
  Matrix matrix[2] = {
      local_matrix_,
      CurrentMatrix(),
  };
  shader_->SetMatrixs(matrix);
  shader_->SetBounds(bounds_[0], bounds_[1]);

  texture_->Bind();
  // TODO maybe use multiple texture channel
  GL_CALL(ActiveTexture, GL_TEXTURE0);
  shader_->SetTextureChannel(0);
}

void GLFillTextureOp::OnAfterDraw(bool has_clip) {
  GLDrawMeshOpAA::OnAfterDraw(has_clip);
  texture_->UnBind();
}

}  // namespace skity
