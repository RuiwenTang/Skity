// this need to be first include
#include "test/common/test_common.hpp"
//

#include <glm/gtc/matrix_transform.hpp>
#include <skity/codec/codec.hpp>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

#include "skity_config.hpp"
#include "src/render/gl/gl_draw_op2.hpp"
#include "src/render/gl/gl_fill2.hpp"
#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_stroke2.hpp"
#include "src/render/gl/gl_texture.hpp"
#include "src/render/gl/gl_vertex.hpp"

class TestGLFill2 : public test::TestApp {
 public:
  TestGLFill2() : test::TestApp(800, 800) {}
  ~TestGLFill2() override = default;

 protected:
  void OnInit() override {
    mvp_ = glm::ortho<float>(0, 800, 800, 0, -100, 100);
    InitGL();
    InitTexture();
    InitMesh();
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //    DrawMesh();

    mesh_->BindMesh();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    shader_->Bind();
    shader_->SetMVPMatrix(mvp_);

    draw_op_->Draw();

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

    //    path.moveTo(100, 100);
    //    path.lineTo(300, 120);
    //    path.lineTo(80, 200);
    //    path.lineTo(300, 250);
    //    path.close();

    path.moveTo(50, 50);
    path.cubicTo(256, 64, 10, 192, 250, 250);
    path.close();

    skity::GLFill2 gl_fill{paint, &gl_vertex};

    range_ = gl_fill.VisitPath(path, true);

    skity::Paint aa_paint{paint};
    aa_paint.setAntiAlias(true);
    aa_paint.setStrokeWidth(2.f);
    aa_paint.setStrokeMiter(2.4f);
    aa_paint.setStrokeCap(skity::Paint::kButt_Cap);
    aa_paint.setStrokeJoin(skity::Paint::kMiter_Join);
    skity::GLStroke2 gl_stroke{aa_paint, &gl_vertex};

    auto aa_range = gl_stroke.VisitPath(path, false);

    range_.aa_outline_start = aa_range.front_start;
    range_.aa_outline_count = aa_range.front_count;
    range_.quad_front_range = aa_range.quad_front_range;

    mesh_->Init();
    mesh_->BindMesh();

    auto vertex_data = gl_vertex.GetVertexDataSize();
    auto front_data = gl_vertex.GetFrontDataSize();
    auto back_data = gl_vertex.GetBackDataSize();
    auto aa_data = gl_vertex.GetAADataSize();
    auto quad_data = gl_vertex.GetQuadDataSize();

    if (std::get<1>(vertex_data) > 0) {
      mesh_->UploadVertexBuffer(std::get<0>(vertex_data),
                                std::get<1>(vertex_data));
    }

    if (std::get<1>(front_data) > 0) {
      mesh_->UploadFrontIndex(std::get<0>(front_data), std::get<1>(front_data));
    }

    if (std::get<1>(back_data) > 0) {
      mesh_->UploadBackIndex(std::get<0>(back_data), std::get<1>(back_data));
    }

    if (std::get<1>(aa_data) > 0) {
    }

    if (std::get<1>(quad_data) > 0) {
      mesh_->UploadQuadIndex(std::get<0>(quad_data), std::get<1>(quad_data));
    }

    draw_op_ = std::make_unique<skity::GLDrawOpFill>(shader_.get(), mesh_.get(),
                                                     range_, true);
    draw_op_->SetAAWidth(2.f);
    draw_op_->SetColorType(skity::GLUniverseShader::kTexture);
    draw_op_->SetUserColor({1.f, 1.f, 1.f, .5f});
    draw_op_->SetGLTexture(texture_);
    draw_op_->SetUserData4({50, 50, 250, 250});
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
  }

  void DrawQuadIfNeed() {
    if (range_.quad_front_range.empty()) {
      return;
    }

    mesh_->BindQuadIndex();

    for (const auto& quad : range_.quad_front_range) {
      shader_->SetUserData2(skity::Vec4{30.f, quad.offset, quad.start});
      shader_->SetUserData3(skity::Vec4{quad.control, quad.end});

      glDrawElements(GL_TRIANGLES, quad.quad_count, GL_UNSIGNED_INT,
                     (void*)(quad.quad_start * sizeof(GLuint)));
    }
  }

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

    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    glStencilFunc(GL_ALWAYS, 0x01, 0xFF);

    skity::GLMeshDraw2 draw_back2{GL_TRIANGLES, range_.back_start,
                                  range_.back_count};

    mesh_->BindBackIndex();
    draw_back2();

    glColorMask(1, 1, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0x00, 0xFF);

    // draw geometry

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0xFF);
    shader_->SetUserData1({skity::GLUniverseShader::kPureColor, 0, 0, 0});

    mesh_->BindFrontIndex();
    draw2();
    DrawQuadIfNeed();

    mesh_->BindBackIndex();
    draw_back2();

    mesh_->UnBindMesh();

    shader_->UnBind();
  }

 private:
  std::unique_ptr<skity::GLUniverseShader> shader_;
  std::unique_ptr<skity::GLMesh> mesh_;
  glm::mat4 mvp_{};
  skity::GLMeshRange range_{};
  std::unique_ptr<skity::GLDrawOp2> draw_op_;
  std::unique_ptr<skity::GLTextureManager> texture_manager_;
  const skity::GLTexture* texture_ = nullptr;
};

int main(int argc, const char** argv) {
  TestGLFill2 fill2;
  fill2.Start();
  return 0;
}