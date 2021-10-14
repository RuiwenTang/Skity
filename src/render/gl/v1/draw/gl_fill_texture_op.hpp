#ifndef SKITY_SRC_RENDER_GL_DRAW_GL_FILL_TEXTURE_OP_HPP
#define SKITY_SRC_RENDER_GL_DRAW_GL_FILL_TEXTURE_OP_HPP

#include <array>
#include <skity/geometry/point.hpp>

#include "gl_draw_mesh_op_aa.hpp"

namespace skity {

class GLTextureShader;
class GLTexture;

class GLFillTextureOp : public GLDrawMeshOpAA {
 public:
  GLFillTextureOp(uint32_t front_start, uint32_t front_count,
                  uint32_t back_start, uint32_t back_count, uint32_t aa_start,
                  uint32_t aa_count, GLTextureShader* shader, GLMesh* mesh);
  ~GLFillTextureOp() override = default;

  void SetLocalMatrix(Matrix const& matrix);

  void SetBounds(Point const& p1, Point const& p2);

  void SetTexture(const GLTexture* texture);

 protected:
  void OnBeforeDraw(bool has_clip) override;
  void OnAfterDraw(bool has_clip) override;
 private:
  GLTextureShader* shader_ = nullptr;
  std::array<Point, 2> bounds_ = {};
  Matrix local_matrix_;
  const GLTexture* texture_ = nullptr;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_DRAW_GL_FILL_TEXTURE_OP_HPP
