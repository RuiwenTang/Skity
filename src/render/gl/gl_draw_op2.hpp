
#ifndef SKITY_SRC_RENDER_GL_GL_DRAW_OP2_HPP
#define SKITY_SRC_RENDER_GL_GL_DRAW_OP2_HPP

#include <memory>
#include <skity/geometry/point.hpp>
#include <skity/graphic/color.hpp>
#include <vector>

#include "src/render/gl/gl_vertex.hpp"
#include "src/utils/lazy.hpp"

namespace skity {

class GLUniverseShader;
class GLMesh;
class GLTexture;

class GLDrawOp2 {
 public:
  GLDrawOp2(GLUniverseShader* shader, GLMesh* mesh, GLMeshRange range);
  virtual ~GLDrawOp2();

  void Draw(bool has_clip);

  void SetUserColor(glm::vec4 const& color4f);
  void SetUserColor(Color color);
  void SetUserData1(glm::ivec4 const& value);
  void SetUserData2(glm::vec4 const& value);
  void SetUserData3(glm::vec4 const& value);
  void SetUserData4(glm::vec4 const& value);
  void SetGLTexture(const GLTexture* texture);
  void SetAAWidth(float width) { aa_width_ = width; }
  void SetColorType(int32_t type) { color_type_ = type; }
  void SetGradientColors(std::vector<glm::vec4> const& colors);
  void SetGradientStops(std::vector<float> const& stops);

 protected:
  virtual void OnDraw(bool has_clip) = 0;

  void UpdateShaderColorType(int32_t type);

  GLUniverseShader* Shader() const { return shader_; }
  GLMeshRange const& Range() const { return range_; }
  float GetAAWidth() const { return aa_width_; }
  int32_t GetColorType() const { return color_type_; }

  void DrawFront();
  void DrawBack();
  void DrawAAOutline();
  void DrawQuadStroke(float stroke_width);

 private:
  GLUniverseShader* shader_;
  GLMesh* mesh_;
  GLMeshRange range_;
  float aa_width_ = 1.f;
  int32_t color_type_ = 0;
  Lazy<glm::vec4> user_color_ = {};
  Lazy<glm::ivec4> user_data1_ = {};
  Lazy<glm::vec4> user_data2_ = {};
  Lazy<glm::vec4> user_data3_ = {};
  Lazy<glm::vec4> user_data4_ = {};
  const GLTexture* gl_texture_ = nullptr;
  std::vector<glm::vec4> gradient_colors_ = {};
  std::vector<float> gradient_stops_ = {};
};

class GLDrawOpFill : public GLDrawOp2 {
 public:
  GLDrawOpFill(GLUniverseShader* shader, GLMesh* mesh, GLMeshRange range,
               bool need_aa);
  ~GLDrawOpFill() override = default;

 protected:
  void OnDraw(bool has_clip) override;

 private:
  bool need_aa_;
};

class GLDrawOpStroke : public GLDrawOp2 {
 public:
  GLDrawOpStroke(GLUniverseShader* shader, GLMesh* mesh, GLMeshRange range,
                 float stroke_width, bool need_aa);
  ~GLDrawOpStroke() override = default;

 protected:
  void OnDraw(bool has_clip) override;

 private:
  float stroke_width_;
  bool need_aa_;
};

class GLDrawOpClip : public GLDrawOp2 {
 public:
  GLDrawOpClip(GLUniverseShader* shader, GLMesh* mesh, GLMeshRange range,
               bool undo);

  ~GLDrawOpClip() override = default;

 protected:
  void OnDraw(bool has_clip) override;

  bool IsUndo() const { return is_undo_; }

 private:
  bool is_undo_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_DRAW_OP2_HPP
