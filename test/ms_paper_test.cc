#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "common/test_common.hpp"
#include "src/geometry/geometry.hpp"

struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint count = 0;
};

class PaperTest : public test::TestApp {
 public:
  PaperTest() : TestApp() {}

 protected:
  void OnInit() override {
    // init matrix
    mvp_ = glm::ortho<float>(0, 800, 600, 0, -100, 100);
    // init gl context
    InitGL();
  }
  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_);
    glUniformMatrix4fv(mvp_location_, 1, GL_FALSE, &mvp_[0][0]);

    glBindVertexArray(mesh_.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_.vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::mat4 m = glm::translate(glm::identity<glm::mat4>(), {300, 0, 0});
    glm::mat4 mm = mvp_ * m;

    glUniformMatrix4fv(mvp_location_, 1, GL_FALSE, &mm[0][0]);

    glBindVertexArray(mesh_cubic_.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_cubic_.vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, mesh_cubic_.count);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);
  }

  void InitGL() {
    // clear color
    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    // init vao and vbo
    std::vector<float> point_data = {
        10.f,  10.f,  0.f, 0.f,   // point 1
        256.f, 64.f,  0.f, 0.5f,  // point 2
        128.f, 128.f, 1.f, 1.f,   // point 3
    };

    glGenVertexArrays(1, &mesh_.vao);
    glGenBuffers(1, &mesh_.vbo);
    mesh_.count = 3;

    glBindVertexArray(mesh_.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_.vbo);
    glBufferData(GL_ARRAY_BUFFER, point_data.size() * sizeof(float),
                 point_data.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::vector<skity::Point> cubics = {
        skity::Point{10.f, 10.f, 0.f, 1.f},
        skity::Point{10.f, 200.f, 0.f, 1.f},
        skity::Point{200.f, 50.f, 0.f, 1.f},
        skity::Point{200.f, 200.f, 0.f, 1.f},
    };

    std::array<skity::Point, 32> sub_cubics;
    skity::SubDividedCubic8(cubics.data(), sub_cubics.data());
    skity::Point start = cubics.front();
    skity::Point center = sub_cubics[sub_cubics.size() / 2];
    skity::Point end = cubics.back();
    std::vector<float> cubic_vertices;
    for (uint32_t i = 0; i < 8; i++) {
      std::array<skity::Point, 3> quad;
      skity::CubicToQuadratic(sub_cubics.data() + i * 4, quad.data());
      cubic_vertices.emplace_back(quad[0].x);
      cubic_vertices.emplace_back(quad[0].y);
      cubic_vertices.emplace_back(0.f);
      cubic_vertices.emplace_back(0.f);

      cubic_vertices.emplace_back(quad[1].x);
      cubic_vertices.emplace_back(quad[1].y);
      cubic_vertices.emplace_back(0.f);
      cubic_vertices.emplace_back(0.5f);

      cubic_vertices.emplace_back(quad[2].x);
      cubic_vertices.emplace_back(quad[2].y);
      cubic_vertices.emplace_back(1.f);
      cubic_vertices.emplace_back(1.f);
    }

    glGenVertexArrays(1, &mesh_cubic_.vao);
    glGenBuffers(1, &mesh_cubic_.vbo);
    mesh_cubic_.count = cubic_vertices.size() / 4;

    glBindVertexArray(mesh_cubic_.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_cubic_.vbo);
    glBufferData(GL_ARRAY_BUFFER, cubic_vertices.size() * sizeof(float),
                 cubic_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // init shader
    const char* vs_code = R"(
      #version 330 core
      layout(location = 0) in vec2 aPos;
      layout(location = 1) in vec2 aUV;
      uniform mat4 mvp;

      out vec2 TexCoord;

      void main() {
        gl_Position = mvp * vec4(aPos, 0.0, 1.0);
        TexCoord = aUV;
      }
    )";

    const char* fs_code = R"(
      #version 330 core

      in vec2 TexCoord;

      out vec4 FragColor;

      void main() {
        if (TexCoord.y * TexCoord.y - TexCoord.x > 0) {
          discard;
        }
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
      }
    )";

    program_ = test::create_shader_program(vs_code, fs_code);
    mvp_location_ = glGetUniformLocation(program_, "mvp");
  }

 private:
  glm::mat4 mvp_ = {};
  Mesh mesh_ = {};
  Mesh mesh_cubic_ = {};
  GLuint program_ = 0;
  GLint mvp_location_ = -1;
};

int main(int argc, const char** argv) {
  PaperTest test;
  test.Start();
  return 0;
}