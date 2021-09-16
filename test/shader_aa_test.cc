
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "common/test_common.hpp"
#include "src/geometry/geometry.hpp"

class ShaderAATest : public test::TestApp {
 public:
  ShaderAATest() : TestApp(800, 800) {}
  ~ShaderAATest() override = default;

 protected:
  void OnInit() override { InitGL(); }
  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program_);
    // mvp
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp_[0][0]);
    // strokeWidth
    //    glUniform1f(stroke_width_location, 20.f);
    // user_color
    glUniform4f(user_color_location, 1.f, 0.f, 0.f, 1.f);

    glBindVertexArray(buffers_[0]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers_[1]);

    // aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void*)0);
    //    // aNormal1
    //    glEnableVertexAttribArray(1);
    //    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
    //                          (void*)(3 * sizeof(float)));
    //    // aNormal2
    //    glEnableVertexAttribArray(2);
    //    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
    //                          (void*)(5 * sizeof(float)));

    // index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
  }

 private:
  void InitGL() {
    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    InitShader();
    InitMesh();
  }

  void InitShader() {
    const char* vs_shader = R"(

#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 mvp;

out float vDir;

void main() {

  gl_Position = mvp * vec4(aPos.xy , 0.0, 1.0);

  vDir = aPos.z;
}

)";

    const char* fs_shader = R"(

#version 330 core

out vec4 FragColor;

uniform vec4 user_color;

in float vDir;

void main() {
  vec4 vColor = user_color;
  // premultiply color
  vColor = vec4(vColor.xyz * vColor.w, vColor.w);

  float feather = abs(vDir);
  if (feather >= 0.9) {
    float alpha = (1.0 - feather) / 0.1;
    vColor *= alpha;
  }

  FragColor = vColor;
}
)";

    program_ = test::create_shader_program(vs_shader, fs_shader);

    mvp_location = glGetUniformLocation(program_, "mvp");
    //    stroke_width_location = glGetUniformLocation(program_, "strokeWidth");
    user_color_location = glGetUniformLocation(program_, "user_color");

    mvp_ = glm::ortho<float>(0, 800, 800, 0, -100, 100);
  }

  void InitMesh() {
    glGenVertexArrays(1, buffers_.data());
    glBindVertexArray(buffers_[0]);

    glGenBuffers(2, buffers_.data() + 1);

    std::vector<float> vertex_buffer;
    std::vector<uint32_t> index_buffer;

    glm::vec2 p1{100, 100};
    glm::vec2 p2{300, 120};
    glm::vec2 p3{200, 400};

    glm::vec2 d = glm::normalize(p2 - p1);
    glm::vec2 n = {-d.y, d.x};

    float stroke_radius = stroke_width * 0.5f;

    glm::vec2 v1 = p1 + n * stroke_radius;
    glm::vec2 v2 = p1 - n * stroke_radius;

    // 0
    vertex_buffer.emplace_back(v1.x);
    vertex_buffer.emplace_back(v1.y);
    vertex_buffer.emplace_back(1.f);
    // 1
    vertex_buffer.emplace_back(v2.x);
    vertex_buffer.emplace_back(v2.y);
    vertex_buffer.emplace_back(-1.f);

    glm::vec2 d2 = glm::normalize(p3 - p2);
    glm::vec2 n2 = glm::vec2(-d2.y, d2.x);

    glm::vec2 o_p2 = p2 + n2 * stroke_radius;
    glm::vec2 i_p2 = p2 - n2 * stroke_radius;

    glm::vec2 vo_p2;
    glm::vec2 vi_p2;
    skity::IntersectLineLine(v1, v1 + d, o_p2, o_p2 + d2, vo_p2);
    skity::IntersectLineLine(v2, v2 + d, i_p2, i_p2 + d2, vi_p2);

    // 2
    vertex_buffer.emplace_back(vo_p2.x);
    vertex_buffer.emplace_back(vo_p2.y);
    vertex_buffer.emplace_back(1.f);
    // 3
    vertex_buffer.emplace_back(vi_p2.x);
    vertex_buffer.emplace_back(vi_p2.y);
    vertex_buffer.emplace_back(-1.f);

    glm::vec2 o_p3 = p3 + n2 * stroke_radius;
    glm::vec2 i_p3 = p3 - n2 * stroke_radius;

    // 4
    vertex_buffer.emplace_back(o_p3.x);
    vertex_buffer.emplace_back(o_p3.y);
    vertex_buffer.emplace_back(1.f);
    // 5
    vertex_buffer.emplace_back(i_p3.x);
    vertex_buffer.emplace_back(i_p3.y);
    vertex_buffer.emplace_back(-1.f);

    index_buffer.emplace_back(0);
    index_buffer.emplace_back(1);
    index_buffer.emplace_back(3);
    index_buffer.emplace_back(0);
    index_buffer.emplace_back(2);
    index_buffer.emplace_back(3);

    index_buffer.emplace_back(2);
    index_buffer.emplace_back(3);
    index_buffer.emplace_back(5);
    index_buffer.emplace_back(2);
    index_buffer.emplace_back(5);
    index_buffer.emplace_back(4);

    glBindBuffer(GL_ARRAY_BUFFER, buffers_[1]);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer.size() * sizeof(float),
                 vertex_buffer.data(), GL_STATIC_DRAW);

    uint32_t err = glGetError();
    std::cout << "error = " << std::hex << err << std::endl;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 index_buffer.size() * sizeof(uint32_t), index_buffer.data(),
                 GL_STATIC_DRAW);

    count = index_buffer.size();
  }

 private:
  GLuint program_ = 0;
  std::array<GLuint, 3> buffers_{};
  glm::mat4 mvp_{};
  int32_t mvp_location = -1;
  int32_t stroke_width_location = -1;
  int32_t user_color_location = -1;
  uint32_t count = 0;
  float stroke_width = 20.f;
};

int main(int argc, const char** argv) {
  ShaderAATest app{};
  app.Start();
  return 0;
}