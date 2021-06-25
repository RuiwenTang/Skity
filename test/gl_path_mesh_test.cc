#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/geometry/point.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <vector>

#include "common/test_common.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_path_visitor.hpp"
#include "src/render/gl/gl_vertex.hpp"

class GLPathMeshDemo : public test::TestApp {
 public:
  GLPathMeshDemo() : TestApp() {}
  ~GLPathMeshDemo() override = default;

 protected:
  void OnInit() override {
    mvp_ = glm::ortho<float>(0, 800, 600, 0, -100, 100) *
           glm::scale(glm::identity<glm::mat4>(), glm::vec3{3.f, 3.f, 1.f}) *
           glm::translate(glm::identity<glm::mat4>(), glm::vec3(50, 150, 0));

    InitGL();
  }
  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);

    glUseProgram(stencil_program_);
    glUniformMatrix4fv(stencil_program_mvp_location_, 1, GL_FALSE, &mvp_[0][0]);

    mesh_.BindMesh();
    glColorMask(0, 0, 0, 0);
    glStencilMask(0x0F);

    glStencilFunc(GL_ALWAYS, 0x01, 0x0F);

    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    mesh_.BindFrontIndex();
    DrawFront(path_1_range);
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    mesh_.BindBackIndex();
    DrawBack(path_1_range);

    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    mesh_.BindFrontIndex();
    DrawFront(path_1_range);
    mesh_.BindBackIndex();
    DrawBack(path_1_range);

    glDisable(GL_STENCIL_TEST);

    glUseProgram(mesh_program_);
    glUniformMatrix4fv(mesh_program_mvp_location_, 1, GL_FALSE, &mvp_[0][0]);

    mesh_.BindFrontIndex();
    DrawFront(path_1_range, GL_LINE_LOOP);
    mesh_.BindBackIndex();
    DrawBack(path_1_range, GL_LINE_LOOP);

    mesh_.UnBindMesh();
  }

  void DrawFront(skity::GLMeshRange const& range, GLenum mode = GL_TRIANGLES) {
    if (range.front_count == 0) {
      return;
    }
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawElements(mode, range.front_count, GL_UNSIGNED_INT,
                   (void*)(range.front_start * sizeof(GLuint)));
  }

  void DrawBack(skity::GLMeshRange const& range, GLenum mode = GL_TRIANGLES) {
    if (range.back_count == 0) {
      return;
    }
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawElements(mode, range.back_count, GL_UNSIGNED_INT,
                   (void*)(range.back_start * sizeof(GLuint)));
  }

  void InitGL() {
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClearStencil(0x00);

    InitShader();
    InitMesh();
  }

  void InitShader() {
    const char* stencil_vs_code = R"(
      #version 330 core
      layout(location = 0) in vec2 aPos;
      layout(location = 1) in vec3 aUV;
      uniform mat4 mvp;

      out vec3 TexCoord;

      void main() {
        gl_Position = mvp * vec4(aPos, 0.0, 1.0);
        TexCoord = aUV;
      }
    )";

    const char* stencil_fs_code = R"(
      #version 330 core

      in vec3 TexCoord;

      out vec4 FragColor;

      void main() {
        if (TexCoord.x == 1.0 && (TexCoord.y * TexCoord.y - TexCoord.z > 0)) {
          discard;
        }
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
      }
    )";

    stencil_program_ =
        test::create_shader_program(stencil_vs_code, stencil_fs_code);

    stencil_program_mvp_location_ =
        glGetUniformLocation(stencil_program_, "mvp");

    const char* mesh_vs_code = R"(
      #version 330 core
      layout(location = 0) in vec2 aPos;

      uniform mat4 mvp;
      void main() {
        gl_Position = mvp * vec4(aPos, 0.0, 1.0);
      }
    )";

    const char* mesh_fs_code = R"(
      #version 330 core

      out vec4 FragColor;

      void main() {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
      } 
    )";

    mesh_program_ = test::create_shader_program(mesh_vs_code, mesh_fs_code);
    mesh_program_mvp_location_ = glGetUniformLocation(mesh_program_, "mvp");
  }

  void InitMesh() {
    skity::Paint paint;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(5.f);
    paint.setStrokeCap(skity::Paint::kRound_Cap);
    paint.setStrokeJoin(skity::Paint::kRound_Join);

    skity::Path path;
    // path.moveTo(10, 10);
    // path.quadTo(256, 64, 128, 128);
    // path.quadTo(10, 192, 250, 250);
    // path.close();
    path.moveTo(88.3281, -127);
    path.quadTo(113.922, -127, 125.688, -116.797);
    path.quadTo(137.469, -106.594, 137.469, -83.1406);
    path.lineTo(135.938, -62.9844);
    path.lineTo(126.969, 0);
    path.lineTo(91.6406, 0);
    path.lineTo(93.9531, -15.5938);
    path.lineTo(93.4375, -15.5938);
    path.quadTo(78.3438, 3, 51.2031, 3);
    path.quadTo(33.5312, 3, 22.7812, -6.1875);
    path.quadTo(12.0312, -15.3906, 12.0312, -29.7031);
    path.quadTo(12.0312, -44.5156, 21.25, -56.3906);
    path.quadTo(30.4688, -68.2656, 49.1562, -73.625);
    path.quadTo(67.8438, -79, 88.3125, -79);
    path.lineTo(102.656, -79);
    path.lineTo(102.906, -82.7812);
    path.quadTo(102.906, -90.0781, 97.0156, -95.4844);
    path.quadTo(91.125, -100.891, 81.4062, -100.891);
    path.quadTo(63.4844, -100.891, 48.125, -86);
    path.lineTo(30.4688, -107.016);
    path.quadTo(54.5312, -127, 88.3281, -127);
    path.moveTo(49.4062, -34.8125);
    path.quadTo(49.4062, -29.4688, 54.1406, -26.2812);
    path.quadTo(58.875, -23.1094, 65.7969, -23.1094);
    path.quadTo(77.0625, -23.1094, 86.5312, -28.7031);
    path.quadTo(96, -34.3125, 98.0469, -46.2812);
    path.lineTo(99.3281, -54.4219);
    path.lineTo(90.375, -54.4219);
    path.quadTo(49.4062, -54.4219, 49.4062, -34.8125);
    path.close();

    skity::Path path2;

    path2.moveTo(400, 10);
    path2.lineTo(500, 10);
    path2.lineTo(400, 110);
    path2.lineTo(500, 110);
    path2.close();

    // skity::Path path3;
    // path3.moveTo(20, 170);
    // path3.conicTo(80, 170, 80, 230, 0.707f);
    // path3.conicTo(80, 170, 20, 170, 0.25f);
    // path3.close();

    skity::GLVertex gl_vertex;
    path_1_range = skity::GLPathVisitor::VisitPath(path, &gl_vertex);
    path_2_range = skity::GLPathVisitor::VisitPath(path2, &gl_vertex);
    // skity::GLPathVisitor::VisitPath(path3, &gl_vertex);

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
  GLuint stencil_program_ = 0;
  GLint stencil_program_mvp_location_ = -1;

  GLuint mesh_program_ = 0;
  GLint mesh_program_mvp_location_ = -1;

  skity::GLMeshRange path_1_range;
  skity::GLMeshRange path_2_range;
};

int main(int argc, const char** argv) {
  GLPathMeshDemo demo;
  demo.Start();
  return 0;
}