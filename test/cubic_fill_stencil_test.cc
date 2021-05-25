#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/geometry/point.hpp>
#include <vector>

#include "common/test_common.hpp"
#include "src/geometry/geometry.hpp"

struct RenderMesh {
  GLuint vao = 0;
  // [vertex, front_index, back_index]
  std::array<GLuint, 3> buffers = {};
  uint32_t front_count = 0;
  uint32_t back_count = 0;

  void BeforeDraw() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  }

  void DrawFront(GLenum mode = GL_TRIANGLES) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawElements(mode, front_count, GL_UNSIGNED_INT, 0);
  }
  void DrawBack(GLenum mode = GL_TRIANGLES) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glDrawElements(mode, back_count, GL_UNSIGNED_INT, 0);
  }
};

class CubicFill : public test::TestApp {
 public:
  CubicFill() : test::TestApp() {}
  ~CubicFill() override = default;

 protected:
  void OnInit() override {
    mvp_ = glm::ortho<float>(0, 800, 600, 0, -100, 100);
    InitGL();
  }
  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);

    glUseProgram(stencil_program_);
    glUniformMatrix4fv(stencil_program_mvp_location_, 1, GL_FALSE, &mvp_[0][0]);

    mesh_.BeforeDraw();
    glColorMask(0, 0, 0, 0);
    glStencilMask(0x0F);

    glStencilFunc(GL_ALWAYS, 0x01, 0x0F);

    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    mesh_.DrawFront();
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    mesh_.DrawBack();

    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    mesh_.DrawFront();
    mesh_.DrawBack();

    glDisable(GL_STENCIL_TEST);

    glUseProgram(mesh_program_);
    glUniformMatrix4fv(mesh_program_mvp_location_, 1, GL_FALSE, &mvp_[0][0]);

    mesh_.DrawFront(GL_LINE_LOOP);
    mesh_.DrawBack(GL_LINE_LOOP);
  }

 private:
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
    // {x,y, type, u ,v}
    std::vector<float> vertices_buffer;
    std::vector<uint32_t> front_index;
    std::vector<uint32_t> back_index;

    std::vector<skity::Point> cubics = {
        skity::Point{10.f, 10.f, 0.f, 1.f},
        skity::Point{10.f, 400.f, 0.f, 1.f},
        skity::Point{400.f, 100.f, 0.f, 1.f},
        skity::Point{400.f, 400.f, 0.f, 1.f},
    };

    std::array<skity::Point, 32> sub_cubics;
    skity::SubDividedCubic8(cubics.data(), sub_cubics.data());

    skity::Point start = cubics.front();
    vertices_buffer.emplace_back(start.x);  // x
    vertices_buffer.emplace_back(start.y);  // y
    vertices_buffer.emplace_back(0);        // type
    vertices_buffer.emplace_back(0);        // u
    vertices_buffer.emplace_back(0);        // v
    uint32_t start_index = 0;
    for (int i = 0; i < 8; i++) {
      std::array<skity::Point, 3> quad;
      skity::CubicToQuadratic(sub_cubics.data() + i * 4, quad.data());
      uint32_t q0_index = vertices_buffer.size() / 5;

      vertices_buffer.emplace_back(quad[0].x);
      vertices_buffer.emplace_back(quad[0].y);
      vertices_buffer.emplace_back(0);
      vertices_buffer.emplace_back(0);
      vertices_buffer.emplace_back(0);

      vertices_buffer.emplace_back(quad[2].x);
      vertices_buffer.emplace_back(quad[2].y);
      vertices_buffer.emplace_back(0);
      vertices_buffer.emplace_back(0);
      vertices_buffer.emplace_back(0);

      uint32_t quad0_index = vertices_buffer.size() / 5;

      vertices_buffer.emplace_back(quad[0].x);
      vertices_buffer.emplace_back(quad[0].y);
      vertices_buffer.emplace_back(1.f);
      vertices_buffer.emplace_back(0.f);
      vertices_buffer.emplace_back(0.f);

      vertices_buffer.emplace_back(quad[1].x);
      vertices_buffer.emplace_back(quad[1].y);
      vertices_buffer.emplace_back(1.f);
      vertices_buffer.emplace_back(0.5f);
      vertices_buffer.emplace_back(0.f);

      vertices_buffer.emplace_back(quad[2].x);
      vertices_buffer.emplace_back(quad[2].y);
      vertices_buffer.emplace_back(1.f);
      vertices_buffer.emplace_back(1.f);
      vertices_buffer.emplace_back(1.f);

      if (skity::CalculateOrientation(start, quad[0], quad[2]) !=
          skity::Orientation::kClockWise) {
        // front
        front_index.emplace_back(start_index);
        front_index.emplace_back(q0_index);
        front_index.emplace_back(q0_index + 1);

        front_index.emplace_back(quad0_index);
        front_index.emplace_back(quad0_index + 1);
        front_index.emplace_back(quad0_index + 2);
      } else {
        // back
        back_index.emplace_back(start_index);
        back_index.emplace_back(q0_index);
        back_index.emplace_back(q0_index + 1);

        back_index.emplace_back(quad0_index);
        back_index.emplace_back(quad0_index + 1);
        back_index.emplace_back(quad0_index + 2);
      }
    }

    mesh_.front_count = front_index.size();
    mesh_.back_count = back_index.size();

    glGenVertexArrays(1, &mesh_.vao);
    glGenBuffers(3, mesh_.buffers.data());

    glBindVertexArray(mesh_.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_.buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices_buffer.size() * sizeof(float),
                 vertices_buffer.data(), GL_STATIC_DRAW);

    // front
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_.buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, front_index.size() * sizeof(uint32_t),
                 front_index.data(), GL_STATIC_DRAW);
    // back
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_.buffers[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, back_index.size() * sizeof(uint32_t),
                 back_index.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

 private:
  RenderMesh mesh_ = {};
  glm::mat4 mvp_ = {};
  GLuint stencil_program_ = 0;
  GLint stencil_program_mvp_location_ = -1;

  GLuint mesh_program_ = 0;
  GLint mesh_program_mvp_location_ = -1;
};

int main(int argc, const char** argv) {
  CubicFill app;
  app.Start();
  return 0;
}