#include <glad/glad.h>
// window
#include <GLFW/glfw3.h>

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/codec/codec.hpp>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <string>

#include "common/test_common.hpp"
#include "skity_config.hpp"
#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_texture.hpp"
#include "src/render/gl/gl_vertex.hpp"
#include "src/render/gl/v1/gl_fill.hpp"
#include "src/render/gl/v1/gl_stroke.hpp"

class TextureTest : public test::TestApp {
 public:
 protected:
  void OnInit() override {
    // manualy init GLInterface
    skity::GLInterface::InitGlobalInterface((void*)glfwGetProcAddress);
    mvp_ = glm::ortho<float>(0, 800, 600, 0, -100, 100);
    InitGL();
    InitMesh();
    InitTexture();
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_STENCIL_TEST);

    stencil_shader->Bind();

    stencil_shader->SetStrokeRadius(5.f);
    stencil_shader->SetMVPMatrix(mvp_);

    mesh_.BindMesh();
    glColorMask(0, 0, 0, 0);
    glStencilMask(0x0F);

    glStencilFunc(GL_ALWAYS, 0x01, 0x0F);

    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    mesh_.BindFrontIndex();
    DrawFront();
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    mesh_.BindBackIndex();
    DrawBack();

    stencil_shader->UnBind();

    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    std::array<skity::Matrix, 2> matris{glm::identity<skity::Matrix>(),
                                        glm::identity<skity::Matrix>()};

    // color_shader->Bind();
    // color_shader->SetMVPMatrix(mvp_);
    // color_shader->SetColor(1.f, 1.f, 0.f, 1.f);

    texture_->Bind();
    glActiveTexture(GL_TEXTURE0);
    texture_shader_->Bind();
    texture_shader_->SetMVPMatrix(mvp_);
    texture_shader_->SetBounds(skity::Point{200, 200, 0, 1},
                               skity::Point{400, 400, 0, 1});
    texture_shader_->SetMatrixs(matris.data());
    texture_shader_->SetTextureChannel(0);

    mesh_.BindFrontIndex();
    DrawFront();
    mesh_.BindBackIndex();
    DrawBack();

    mesh_.UnBindMesh();
    // color_shader->UnBind();
    texture_shader_->UnBind();
  }

 private:
  void InitGL() {
    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glClearStencil(0);

    texture_shader_ = skity::GLShader::CreateTextureShader();
    stencil_shader = skity::GLShader::CreateStencilShader();
    color_shader = skity::GLShader::CreateColorShader();
  }

  void InitMesh() {
    skity::GLVertex gl_vertex;

    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(10.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    // paint.setStrokeJoin(skity::Paint::kMiter_Join);
    paint.setStrokeJoin(skity::Paint::kRound_Join);

    skity::Path path;

    const float R = 100.f, C = 300.0f;
    path.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
      float a = 2.6927937f * i;
      path.lineTo(C + R * std::cos(a), C + R * std::sin(a));
    }
    path.close();

    // skity::GLStroke stroke(paint);
    // mesh_range_ = stroke.strokePath(path, &gl_vertex);
    skity::GLFill fill{};
    mesh_range_ = fill.fillPath(path, paint, &gl_vertex);

    mesh_.Init();
    mesh_.BindMesh();
    mesh_.UploadVertexBuffer(gl_vertex.GetVertexData(),
                             gl_vertex.GetVertexDataSize());
    mesh_.UploadFrontIndex(gl_vertex.GetFrontIndexData(),
                           gl_vertex.GetFrontIndexDataSize());
    mesh_.UploadBackIndex(gl_vertex.GetBackIndexData(),
                          gl_vertex.GetBackIndexDataSize());
    mesh_.UnBindMesh();
  }

  void InitTexture() {
    texture_manager_ = std::make_unique<skity::GLTextureManager>();

    auto skity_data = skity::Data::MakeFromFileName(BUILD_IN_IMAGE_FILE);

    auto codec = skity::Codec::MakeFromData(skity_data);
    if (!codec) {
      exit(-1);
    }
    codec->SetData(skity_data);
    auto pixmap = codec->Decode();
    if (!pixmap) {
      exit(-1);
    }

    texture_ = texture_manager_->GenerateTexture(pixmap.get());

    if (!texture_) {
      exit(-1);
    }
  }

  void DrawFront(GLenum mode = GL_TRIANGLES) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glDrawElements(mode, mesh_range_.front_count, GL_UNSIGNED_INT, 0);
  }

  void DrawBack(GLenum mode = GL_TRIANGLES) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glDrawElements(mode, mesh_range_.back_count, GL_UNSIGNED_INT, 0);
  }

 private:
  skity::Matrix mvp_;
  skity::GLMesh mesh_;
  skity::GLMeshRange mesh_range_;
  const skity::GLTexture* texture_ = nullptr;
  std::shared_ptr<skity::StencilShader> stencil_shader;
  std::unique_ptr<skity::GLTextureShader> texture_shader_;
  std::shared_ptr<skity::ColorShader> color_shader;
  std::unique_ptr<skity::GLTextureManager> texture_manager_;
};

int main(int argc, const char** argv) {
  TextureTest app;
  app.Start();
  return 0;
}
