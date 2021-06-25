#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/geometry/point.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <string>
#include <vector>

#include "common/test_common.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_path_visitor.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_stroke.hpp"
#include "src/render/gl/gl_vertex.hpp"

class GLPathMeshDemo : public test::TestApp {
 public:
  GLPathMeshDemo() : TestApp() {}
  ~GLPathMeshDemo() override = default;

 protected:
  void OnInit() override {
    mvp_ = glm::ortho<float>(0, 800, 600, 0, -100, 100);
    InitGL();
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

    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    mesh_.BindFrontIndex();
    DrawFront();
    mesh_.BindBackIndex();
    DrawBack();

    stencil_shader->UnBind();

    glDisable(GL_STENCIL_TEST);

    color_shader->Bind();
    color_shader->SetMVPMatrix(mvp_);
    color_shader->SetColor(1.f, 1.f, 0.f, 1.f);
    mesh_.BindFrontIndex();
    DrawFront(GL_LINE_LOOP);
    mesh_.BindBackIndex();
    DrawBack(GL_LINE_LOOP);

    mesh_.UnBindMesh();
    color_shader->UnBind();
  }

  void DrawFront(GLenum mode = GL_TRIANGLES) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawElements(mode, mesh_range_.front_count, GL_UNSIGNED_INT, 0);
  }

  void DrawBack(GLenum mode = GL_TRIANGLES) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawElements(mode, mesh_range_.back_count, GL_UNSIGNED_INT, 0);
  }

  void InitGL() {
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClearStencil(0x00);

    InitShader();
    InitMesh();
  }

  void InitShader() {
    stencil_shader = skity::GLShader::CreateStencilShader();
    color_shader = skity::GLShader::CreateColorShader();
  }

  void InitMesh() {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(10.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    // paint.setStrokeJoin(skity::Paint::kMiter_Join);
    paint.setStrokeJoin(skity::Paint::kRound_Join);

    skity::Path path;
    // path.moveTo(100, 100);
    // path.lineTo(180, 200);
    // path.lineTo(300, 100);
    // path.lineTo(400, 200);
    // path.lineTo(360, 100);
    // path.close();
    // path.moveTo(50, 10);
    // path.lineTo(40, 50);
    path.moveTo(50, 50);
    path.cubicTo(256, 64, 10, 192, 250, 250);
    path.close();

    const float R = 100.f, C = 300.0f;
    path.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
      float a = 2.6927937f * i;
      path.lineTo(C + R * std::cos(a), C + R * std::sin(a));
    }
    path.close();

    path.moveTo(71.4311121f, 56.f);
    path.cubicTo(68.6763107f, 56.0058575f, 65.9796704f, 57.5737917f,
                 64.5928855f, 59.965729f);
    path.lineTo(43.0238921f, 97.5342563f);
    path.cubicTo(41.6587026f, 99.9325978f, 41.6587026f, 103.067402f,
                 43.0238921f, 105.465744f);
    path.lineTo(64.5928855f, 143.034271f);
    path.cubicTo(65.9798162f, 145.426228f, 68.6763107f, 146.994582f,
                 71.4311121f, 147.f);
    path.lineTo(114.568946f, 147.f);
    path.cubicTo(117.323748f, 146.994143f, 120.020241f, 145.426228f,
                 121.407172f, 143.034271f);
    path.lineTo(142.976161f, 105.465744f);
    path.cubicTo(144.34135f, 103.067402f, 144.341209f, 99.9325978f, 142.976161f,
                 97.5342563f);
    path.lineTo(121.407172f, 59.965729f);
    path.cubicTo(120.020241f, 57.5737917f, 117.323748f, 56.0054182f,
                 114.568946f, 56.f);
    path.lineTo(71.4311121f, 56.f);
    path.close();

    // path.quadTo(256, 64, 128, 128);
    // path.quadTo(10, 192, 250, 250);

    skity::GLVertex gl_vertex;
    skity::GLStroke stroke(paint);
    mesh_range_ = stroke.strokePath(path, &gl_vertex);

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

 private:
  skity::GLMesh mesh_;
  glm::mat4 mvp_ = {};
  skity::GLMeshRange mesh_range_;
  std::shared_ptr<skity::StencilShader> stencil_shader;
  std::shared_ptr<skity::ColorShader> color_shader;
};

int main(int argc, const char** argv) {
  GLPathMeshDemo demo;
  demo.Start();
  return 0;
}