
#include "src/render/gl/gl_draw_op2.hpp"

#include <utility>

#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"

namespace skity {

GLDrawOp2::GLDrawOp2(GLUniverseShader* shader, GLMesh* mesh, GLMeshRange range)
    : shader_(shader), mesh_(mesh), range_(std::move(range)) {}

GLDrawOp2::~GLDrawOp2() = default;

void GLDrawOp2::Draw(bool has_clip) {
  // user color
  if (user_color_.IsValid()) {
    shader_->SetUserColor(*user_color_);
  }

  // user data1
  if (user_data1_.IsValid()) {
    shader_->SetUserData1(*user_data1_);
  }

  // user data2
  if (user_data2_.IsValid()) {
    shader_->SetUserData2(*user_data2_);
  }

  // user data3
  if (user_data3_.IsValid()) {
    shader_->SetUserData3(*user_data3_);
  }

  this->OnDraw(has_clip);
}

void GLDrawOp2::SetUserColor(Color color) {
  SetUserColor(Color4fFromColor(color));
}

void GLDrawOp2::SetUserColor(const glm::vec4& color4f) {
  user_color_.Set(color4f);
}

void GLDrawOp2::SetUserData1(const glm::ivec4& value) {
  user_data1_.Set(value);
}

void GLDrawOp2::SetUserData2(const glm::vec4& value) { user_data2_.Set(value); }

void GLDrawOp2::SetUserData3(const glm::vec4& value) { user_data3_.Set(value); }

void GLDrawOp2::UpdateShaderColorType(int32_t type) {
  glm::ivec4 value = {};

  if (user_data1_.IsValid()) {
    value = *user_data1_;
  }

  value.x = type;

  Shader()->SetUserData1(value);
}

void GLDrawOp2::DrawFront() {
  if (range_.front_count == 0) {
    return;
  }

  mesh_->BindFrontIndex();
  GLMeshDraw2 drawer{GL_TRIANGLES, range_.front_start, range_.front_count};
  drawer();
}

void GLDrawOp2::DrawBack() {
  if (range_.back_count == 0) {
    return;
  }

  mesh_->BindBackIndex();
  GLMeshDraw2 drawer{GL_TRIANGLES, range_.back_start, range_.back_count};
  drawer();
}

void GLDrawOp2::DrawAAOutline() {
  if (range_.aa_outline_count == 0) {
    return;
  }

  mesh_->BindFrontIndex();
  GLMeshDraw2 drawer{GL_TRIANGLES, range_.aa_outline_start,
                     range_.aa_outline_count};
  drawer();
}

void GLDrawOp2::DrawQuadStroke(float stroke_width) {
  if (range_.quad_front_range.empty()) {
    return;
  }

  mesh_->BindQuadIndex();
  for (const auto& quad : range_.quad_front_range) {
    shader_->SetUserData2(Vec4{stroke_width, quad.offset, quad.start});
    shader_->SetUserData3(Vec4{quad.control, quad.end});

    GLMeshDraw2 drawer{GL_TRIANGLES, quad.quad_start, quad.quad_count};
    drawer();
  }
}

GLDrawOpFill::GLDrawOpFill(GLUniverseShader* shader, GLMesh* mesh,
                           GLMeshRange range, bool need_aa)
    : GLDrawOp2(shader, mesh, std::move(range)), need_aa_(need_aa) {}

void GLDrawOpFill::OnDraw(bool has_clip) {
  // step 1 stencil
  // disable color output
  GL_CALL(ColorMask, 0, 0, 0, 0);
  // stencil mask
  GL_CALL(StencilMask, 0x0F);
  GL_CALL(StencilFunc, GL_ALWAYS, 0x01, 0x0F);
  // update shader type
  UpdateShaderColorType(GLUniverseShader::kStencil);
  // front stencil op
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
  DrawFront();
  // back stencil op
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
  DrawBack();

  // enable color output
  GL_CALL(ColorMask, 1, 1, 1, 1);

  // draw aa outline
  if (need_aa_) {
    UpdateShaderColorType(GetColorType() | GLUniverseShader::kAAOutline);
    Shader()->SetUserData2({GetAAWidth(), 0.f, 0.f, 0.f});
    GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_KEEP);
    if (has_clip) {
      GL_CALL(StencilFunc, GL_EQUAL, 0x10, 0x1F);
    } else {
      GL_CALL(StencilFunc, GL_EQUAL, 0x00, 0x0F);
    }
    DrawAAOutline();
    DrawQuadStroke(GetAAWidth());
  }

  // Fill normal color
  UpdateShaderColorType(GetColorType());
  // change stencil op
  // we need to replace stencil to make sure no fragment overlap
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);
  if (has_clip) {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x10, 0x1F);
  } else {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x00, 0x0F);
  }

  DrawFront();
  DrawBack();
}

GLDrawOpStroke::GLDrawOpStroke(GLUniverseShader* shader, GLMesh* mesh,
                               GLMeshRange range, float stroke_width,
                               bool need_aa)
    : GLDrawOp2(shader, mesh, std::move(range)),
      stroke_width_(stroke_width),
      need_aa_(need_aa) {}

void GLDrawOpStroke::OnDraw(bool has_clip) {
  // step 1 stencil
  // disable color output
  GL_CALL(ColorMask, 0, 0, 0, 0);
  // stencil mask
  GL_CALL(StencilMask, 0x0F);
  GL_CALL(StencilFunc, GL_ALWAYS, 0x01, 0x0F);
  // update shader type
  UpdateShaderColorType(GLUniverseShader::kStencil);
  // front stencil op
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_INCR_WRAP);

  // stroke only contains front and quad range
  // setup StrokeWidth
  Shader()->SetUserData2({stroke_width_, 0.f, 0.f, 0.f});
  DrawFront();
  DrawQuadStroke(stroke_width_);

  // enable Color output
  GL_CALL(ColorMask, 1, 1, 1, 1);
  if (need_aa_) {
    GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_KEEP);
    if (has_clip) {
      GL_CALL(StencilFunc, GL_EQUAL, 0x10, 0x1F);
    } else {
      GL_CALL(StencilFunc, GL_EQUAL, 0x00, 0x0F);
    }
    UpdateShaderColorType(GetColorType() | GLUniverseShader::kAAOutline);

    DrawFront();
    DrawQuadStroke(stroke_width_);
  }

  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);
  Shader()->SetUserData1({GetColorType(), 0, 0, 0});
  if (has_clip) {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x10, 0x1F);
  } else {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x00, 0x0F);
  }

  DrawFront();
  DrawQuadStroke(stroke_width_);
}

}  // namespace skity
