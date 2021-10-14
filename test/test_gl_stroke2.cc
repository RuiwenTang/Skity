// this need to be first include
#include "test/common/test_common.hpp"
//

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_vertex.hpp"
#include "src/render/gl/v2/gl_draw_op2.hpp"
#include "src/render/gl/v2/gl_stroke2.hpp"

class TestGLStroke2 : public test::TestApp {
 public:
  TestGLStroke2() : test::TestApp(800, 800) {}

 protected:
  void OnInit() override {
    mvp_ = glm::ortho<float>(0, 800, 800, 0, -100, 100);
    InitGL();
    InitMesh();
  }
  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // DrawMesh();

    mesh_->BindMesh();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    shader_->Bind();
    shader_->SetMVPMatrix(mvp_);

    stroke_op_->Draw();

    shader_->UnBind();
    mesh_->UnBindMesh();
  }

 private:
  void InitGL() {
    skity::GLInterface::InitGlobalInterface((void*)glfwGetProcAddress);

    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glClearStencil(0x00);
    glStencilMask(0xFF);

    // blend is need for anti-alias
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);

    GLint num = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num);
    for (GLint i = 0; i < num; i++) {
      std::cout << glGetStringi(GL_EXTENSIONS, i) << std::endl;
    }

    shader_ = skity::GLShader::CreateUniverseShader();
  }
  void InitMesh() {
    mesh_ = std::make_unique<skity::GLMesh>();

    skity::GLVertex2 gl_vertex;

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(30.f);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setColor(skity::Color_WHITE);

    skity::Path path;

    //    path.lineTo(300, 120);
    //    path.lineTo(130, 300);
    //    path.lineTo(300, 350);
    //    path.lineTo(80, 340);
    //    path.close();

    //    path.quadTo(300, 100, 200, 200);
    path.moveTo(50, 50);
    path.cubicTo(256, 64, 10, 192, 250, 250);
    path.close();

    skity::GLStroke2 gl_stroke{paint, &gl_vertex};

    range_ = gl_stroke.VisitPath(path, false);

    mesh_->Init();
    mesh_->BindMesh();

    auto vertex_data = gl_vertex.GetVertexDataSize();
    auto quad_buffer = gl_vertex.GetQuadBufferDataSize();
    auto front_data = gl_vertex.GetFrontDataSize();
    auto back_data = gl_vertex.GetBackDataSize();
    auto aa_data = gl_vertex.GetAADataSize();
    auto quad_data = gl_vertex.GetQuadDataSize();

    if (std::get<1>(vertex_data) > 0) {
      mesh_->UploadVertexBuffer(std::get<0>(vertex_data),
                                std::get<1>(vertex_data));
    }

    if (std::get<1>(quad_buffer) > 0) {
      mesh_->UploadQuadBuffer(std::get<0>(quad_buffer),
                              std::get<1>(quad_buffer));
    }

    if (std::get<1>(front_data) > 0) {
      mesh_->UploadFrontIndex(std::get<0>(front_data), std::get<1>(front_data));
    }

    if (std::get<1>(back_data) > 0) {
      mesh_->UploadBackIndex(std::get<0>(front_data), std::get<1>(back_data));
    }

    if (std::get<1>(aa_data) > 0) {
    }

    if (std::get<1>(quad_data) > 0) {
      mesh_->UploadQuadIndex(std::get<0>(quad_data), std::get<1>(quad_data));
    }

    stroke_op_ = std::make_unique<skity::GLDrawOpStroke>(
        shader_.get(), mesh_.get(), range_, paint.getStrokeWidth(), true);

    skity::Matrix matrix = glm::identity<glm::mat4>();
    stroke_op_->SetUserTransform(matrix);
    stroke_op_->SetUserShaderMatrix(matrix);
    stroke_op_->SetColorType(skity::GLUniverseShader::kGradientLinear);
    stroke_op_->SetUserData1(
        {skity::GLUniverseShader::kGradientLinear, 0, 2, 0});

    stroke_op_->SetUserData4({50.f, 50.f, 250.f, 250.f});
    std::vector<skity::Vec4> colors{skity::Vec4{1.f, 0.f, 0.f, 1.f},
                                    skity::Vec4{0.f, 1.f, 1.f, 1.f}};
    stroke_op_->SetGradientColors(colors);
    stroke_op_->SetUserColor({1.f, 1.f, 1.f, .5f});
  }

  void DrawQuadIfNeed() {}

  void DrawMesh() {
    shader_->Bind();

    shader_->SetMVPMatrix(mvp_);
    shader_->SetUserColor({1.f, 1.f, 1.f, .5f});
    shader_->SetUserData1({skity::GLUniverseShader::kStencil, 0, 0, 0});
    shader_->SetUserData2({30.f, 0.f, 0.f, 0.f});

    mesh_->BindMesh();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    skity::GLMeshDraw2 draw2{GL_TRIANGLES, range_.front_start,
                             range_.front_count};

    mesh_->BindFrontIndex();

    glColorMask(0, 0, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilFunc(GL_ALWAYS, 0x01, 0xFF);

    draw2();

    DrawQuadIfNeed();

    glColorMask(1, 1, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0x00, 0xFF);

    // draw aa_outline first
    shader_->SetUserData1({skity::GLUniverseShader::kAAOutline, 0, 0, 0});

    mesh_->BindFrontIndex();
    draw2();
    DrawQuadIfNeed();

    // draw geometry

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0xFF);
    shader_->SetUserData1({skity::GLUniverseShader::kPureColor, 0, 0, 0});

    mesh_->BindFrontIndex();
    draw2();
    DrawQuadIfNeed();

    mesh_->UnBindMesh();

    shader_->UnBind();
  }

 private:
  std::unique_ptr<skity::GLUniverseShader> shader_;
  std::unique_ptr<skity::GLMesh> mesh_;
  glm::mat4 mvp_;
  skity::GLMeshRange range_;
  std::unique_ptr<skity::GLDrawOp2> stroke_op_;
};

int main(int argc, const char** argv) {
  TestGLStroke2 app{};
  app.Start();
  return 0;
}