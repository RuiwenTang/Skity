#include "src/render/gl/gl_draw_op.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/geometry/point.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <vector>

#include "common/test_common.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_stroke.hpp"
#include "src/render/gl/gl_vertex.hpp"

class GLDrawOpDemo : public test::TestApp {
 public:
  GLDrawOpDemo() : TestApp() {}
  ~GLDrawOpDemo() override = default;

 protected:
  void OnInit() override {
    InitGL();
    InitPath();
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (const auto& op : draw_ops_) {
      op->Draw();
    }
  }

  void InitGL() {
    stencil_shader_ = skity::GLShader::CreateStencilShader();
    color_shader_ = skity::GLShader::CreateColorShader();
    glClearColor(0.3, 0.4, 0.5, 1.0);
    glClearStencil(0x0);
  }

  void InitPath() {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(10.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);

    skity::Path path;
    path.moveTo(350, 50);
    path.cubicTo(556, 64, 310, 192, 550, 250);

    skity::Path path2;
    const float R = 100.f, C = 300.0f;
    path2.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
      float a = 2.6927937f * i;
      path2.lineTo(C + R * std::cos(a), C + R * std::sin(a));
    }
    path2.close();

    skity::Path path3;

    path3.moveTo(71.4311121f, 56.f);
    path3.cubicTo(68.6763107f, 56.0058575f, 65.9796704f, 57.5737917f,
                  64.5928855f, 59.965729f);
    path3.lineTo(43.0238921f, 97.5342563f);
    path3.cubicTo(41.6587026f, 99.9325978f, 41.6587026f, 103.067402f,
                  43.0238921f, 105.465744f);
    path3.lineTo(64.5928855f, 143.034271f);
    path3.cubicTo(65.9798162f, 145.426228f, 68.6763107f, 146.994582f,
                  71.4311121f, 147.f);
    path3.lineTo(114.568946f, 147.f);
    path3.cubicTo(117.323748f, 146.994143f, 120.020241f, 145.426228f,
                  121.407172f, 143.034271f);
    path3.lineTo(142.976161f, 105.465744f);
    path3.cubicTo(144.34135f, 103.067402f, 144.341209f, 99.9325978f,
                  142.976161f, 97.5342563f);
    path3.lineTo(121.407172f, 59.965729f);
    path3.cubicTo(120.020241f, 57.5737917f, 117.323748f, 56.0054182f,
                  114.568946f, 56.f);
    path3.lineTo(71.4311121f, 56.f);
    path3.close();

    skity::GLVertex gl_vertex;
    skity::GLStroke stroke(paint);
    skity::GLStroke stroke2(paint);
    skity::GLStroke stroke3(paint);

    auto path1_range = stroke.strokePath(path, &gl_vertex);
    auto path2_range = stroke2.strokePath(path2, &gl_vertex);
    auto path3_range = stroke3.strokePath(path3, &gl_vertex);

    mesh_.Init();
    mesh_.BindMesh();
    mesh_.UploadVertexBuffer(gl_vertex.GetVertexData(),
                             gl_vertex.GetVertexDataSize());
    mesh_.UploadFrontIndex(gl_vertex.GetFrontIndexData(),
                           gl_vertex.GetFrontIndexDataSize());
    mesh_.UploadBackIndex(gl_vertex.GetBackIndexData(),
                          gl_vertex.GetBackIndexDataSize());

    auto mvp = glm::ortho<float>(0, 800, 600, 0, -100, 100);
    skity::GLDrawOpBuilder::UpdateMVPMatrix(mvp);

    skity::GLDrawOpBuilder::UpdateStencilShader(stencil_shader_.get());
    skity::GLDrawOpBuilder::UpdateColorShader(color_shader_.get());

    skity::GLDrawOpBuilder::UpdateMesh(&mesh_);

    skity::GLDrawOpBuilder::UpdateFrontStart(path1_range.front_start);
    skity::GLDrawOpBuilder::UpdateFrontCount(path1_range.front_count);
    skity::GLDrawOpBuilder::UpdateBackStart(path1_range.back_start);
    skity::GLDrawOpBuilder::UpdateBackCount(path1_range.back_count);

    draw_ops_.emplace_back(std::move(
        skity::GLDrawOpBuilder::CreateStencilOp(paint.getStrokeWidth())));

    draw_ops_.emplace_back(
        std::move(skity::GLDrawOpBuilder::CreateColorOp(1.f, 1.f, 0.f, 1.f)));

    skity::GLDrawOpBuilder::UpdateFrontStart(path2_range.front_start);
    skity::GLDrawOpBuilder::UpdateFrontCount(path2_range.front_count);
    skity::GLDrawOpBuilder::UpdateBackStart(path2_range.back_start);
    skity::GLDrawOpBuilder::UpdateBackCount(path2_range.back_count);

    draw_ops_.emplace_back(std::move(
        skity::GLDrawOpBuilder::CreateStencilOp(paint.getStrokeWidth())));

    draw_ops_.emplace_back(
        std::move(skity::GLDrawOpBuilder::CreateColorOp(1.f, 0.f, 0.f, 1.f)));

    skity::GLDrawOpBuilder::UpdateFrontStart(path3_range.front_start);
    skity::GLDrawOpBuilder::UpdateFrontCount(path3_range.front_count);
    skity::GLDrawOpBuilder::UpdateBackStart(path3_range.back_start);
    skity::GLDrawOpBuilder::UpdateBackCount(path3_range.back_count);

    draw_ops_.emplace_back(std::move(
        skity::GLDrawOpBuilder::CreateStencilOp(paint.getStrokeWidth())));

    draw_ops_.emplace_back(
        std::move(skity::GLDrawOpBuilder::CreateColorOp(0.f, 0.f, 1.f, 1.f)));
  }

 private:
  std::unique_ptr<skity::StencilShader> stencil_shader_;
  std::unique_ptr<skity::ColorShader> color_shader_;
  skity::GLMesh mesh_;
  std::vector<std::unique_ptr<skity::GLDrawOp>> draw_ops_;
};

int main(int argc, const char** argv) {
  GLDrawOpDemo demo;
  demo.Start();
  return 0;
}