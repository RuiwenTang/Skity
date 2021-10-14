
#include <glad/glad.h>
// window
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <string>

#include "common/test_common.hpp"
#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_vertex.hpp"
#include "src/render/gl/v1/gl_stroke.hpp"

using namespace skity;

class RawGLRenderTest : public test::TestApp {
 public:
  RawGLRenderTest() = default;
  ~RawGLRenderTest() override = default;

 protected:
  void OnInit() override {
    // manualy init GLInterface
    GLInterface::InitGlobalInterface((void*)glfwGetProcAddress);
    mvp_ = glm::ortho<float>(0, 800, 600, 0, -100, 100);
    InitGL();
  }

  void InitGL() {
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClearStencil(0x00);
    stencil_shader_ = GLShader::CreateStencilShader();
    gradient_shader_ = GLShader::CreateGradientShader();
    color_shader_ = GLShader::CreateColorShader();

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    InitMesh();
  }

  void InitMesh() {
    GLVertex gl_vertex;

    int32_t p1 =
        gl_vertex.AddPoint(10, 10, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t p2 =
        gl_vertex.AddPoint(300, 10, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t p3 =
        gl_vertex.AddPoint(300, 300, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    int32_t p4 =
        gl_vertex.AddPoint(10, 300, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    gl_vertex.AddFront(p1, p2, p3);
    gl_vertex.AddFront(p1, p3, p4);

    mesh_.Init();
    mesh_.BindMesh();
    mesh_.UploadVertexBuffer(gl_vertex.GetVertexData(),
                             gl_vertex.GetVertexDataSize());
    mesh_.UploadFrontIndex(gl_vertex.GetFrontIndexData(),
                           gl_vertex.GetFrontIndexDataSize());
    mesh_.UploadBackIndex(gl_vertex.GetBackIndexData(),
                          gl_vertex.GetBackIndexDataSize());
    mesh_.UnBindMesh();

    front_count_ = gl_vertex.FrontCount();
    back_count_ = gl_vertex.BackCount();
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // step 1 stencil path
    glEnable(GL_STENCIL_TEST);

    stencil_shader_->Bind();

    stencil_shader_->SetStrokeRadius(5.f);
    stencil_shader_->SetMVPMatrix(mvp_);

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

    stencil_shader_->UnBind();

    // step 2 draw color
    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // color_shader_->Bind();
    // color_shader_->SetMVPMatrix(mvp_);
    // color_shader_->SetColor(1.f, 1.f, 1.f, 1.f);
    std::vector<Vec4> colors = {
        Vec4{1, 0, 0, 1},
        Vec4{0, 1, 0, 1},
        Vec4{0, 0, 1, 0},
    };
    std::vector<float> stops = {
        0.f,
        0.3f,
        1.f,
    };
    std::array<skity::Matrix, 2> matrixs{};
    matrixs[0] = glm::rotate(glm::identity<glm::mat4>(), glm::radians(45.f),
                             glm::vec3(0, 0, 1));
    matrixs[1] = glm::identity<glm::mat4>();
    gradient_shader_->Bind();
    gradient_shader_->SetMVPMatrix(mvp_);
    gradient_shader_->SetMatrixs(matrixs.data());
    gradient_shader_->SetGradientType(Shader::GradientType::kLinear);
    gradient_shader_->SetPoints(Point{10, 10, 0, 1}, Point{300, 300, 0, 1});
    gradient_shader_->SetColorCount(colors.size());
    gradient_shader_->SetColors(colors);
    gradient_shader_->SetStopCount(stops.size());
    gradient_shader_->SetStops(stops);

    mesh_.BindFrontIndex();
    DrawFront(GL_TRIANGLES);
    mesh_.BindBackIndex();
    DrawBack(GL_TRIANGLES);

    // color_shader_->UnBind();
    gradient_shader_->UnBind();

    mesh_.UnBindMesh();
    gradient_shader_->UnBind();
  }

  void DrawFront(GLenum mode = GL_TRIANGLES) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glDrawElements(mode, front_count_, GL_UNSIGNED_INT, 0);
  }

  void DrawBack(GLenum mode = GL_TRIANGLES) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glDrawElements(mode, back_count_, GL_UNSIGNED_INT, 0);
  }

 private:
  Matrix mvp_;
  std::unique_ptr<StencilShader> stencil_shader_;
  std::unique_ptr<GLGradientShader> gradient_shader_;
  std::unique_ptr<ColorShader> color_shader_;
  GLMesh mesh_;

  uint32_t front_count_ = 0;
  uint32_t back_count_ = 0;
};

int main(int argc, const char** argv) {
  RawGLRenderTest app;
  app.Start();
  return 0;
}
