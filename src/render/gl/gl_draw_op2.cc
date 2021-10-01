
#include "src/render/gl/gl_draw_op2.hpp"

#include <utility>

#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_texture.hpp"

namespace skity {

GLDrawOp2::GLDrawOp2(GLUniverseShader* shader, GLMesh* mesh, GLMeshRange range)
    : shader_(shader), mesh_(mesh), range_(std::move(range)) {}

GLDrawOp2::~GLDrawOp2() = default;

void GLDrawOp2::Draw() {
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

  // user data 4
  if (user_data4_.IsValid()) {
    shader_->SetUserData4(*user_data4_);
  }

  // user transform
  if (user_transform_.IsValid()) {
    shader_->SetUserTransform(*user_transform_);
  }

  // Shader matrix
  if (user_shader_matrix_.IsValid()) {
    shader_->SetUserShaderMatrix(*user_shader_matrix_);
  }

  if (gl_texture_) {
    gl_texture_->Bind();
    GL_CALL(ActiveTexture, GL_TEXTURE0);
    shader_->SetUserTexture(0);
  }

  // gradient data
  if (gradient_colors_.size() >= 2) {
    shader_->SetGradientColors(gradient_colors_);
    if (gradient_stops_.size() >= 2) {
      shader_->SetGradientStops(gradient_stops_);
    }
  }

  this->OnDraw(HasClip());

  if (gl_texture_) {
    gl_texture_->UnBind();
  }
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

void GLDrawOp2::SetUserData4(const glm::vec4& value) { user_data4_.Set(value); }

void GLDrawOp2::SetUserTransform(glm::mat4 const& value) {
  user_transform_.Set(value);
}

void GLDrawOp2::SetUserShaderMatrix(glm::mat4 const& value) {
  user_shader_matrix_.Set(value);
}

void GLDrawOp2::SetGLTexture(const GLTexture* texture) {
  gl_texture_ = texture;
}

void GLDrawOp2::SetGradientColors(const std::vector<glm::vec4>& colors) {
  gradient_colors_ = colors;
}

void GLDrawOp2::SetGradientStops(const std::vector<float>& stops) {
  gradient_stops_ = stops;
}

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
  UpdateShaderColorType(GLUniverseShader::kStencil | GetColorType());
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
  UpdateShaderColorType(GetColorType());
  if (has_clip) {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x10, 0x1F);
  } else {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x00, 0x0F);
  }

  DrawFront();
  DrawQuadStroke(stroke_width_);
}

GLDrawOpClip::GLDrawOpClip(GLUniverseShader* shader, GLMesh* mesh,
                           GLMeshRange range, bool undo)
    : GLDrawOp2(shader, mesh, std::move(range)), is_undo_(undo) {}

void GLDrawOpClip::OnDraw(bool has_clip) {
  // Disable Color Output
  GL_CALL(ColorMask, 0, 0, 0, 0);
  if (!IsUndo()) {
    // Step 1 Stencil Path line Fill Operation
    GL_CALL(StencilMask, 0x0F);
    GL_CALL(StencilFunc, GL_ALWAYS, 0x01, 0x0F);

    // front stencil op
    GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    DrawFront();
    // back stencil op
    GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    DrawBack();
  }

  // Move stencil to high 8 bits all clear
  GL_CALL(StencilMask, 0xFF);
  if (IsUndo()) {
    GL_CALL(StencilFunc, GL_ALWAYS, 0x00, 0x0F);
  } else {
    GL_CALL(StencilFunc, GL_NOTEQUAL, 0x10, 0x0F);
  }
  GL_CALL(StencilOp, GL_KEEP, GL_KEEP, GL_REPLACE);

  DrawFront();
  DrawBack();

  GL_CALL(StencilMask, 0x0F);
}

}  // namespace skity
