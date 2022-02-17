#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "test/common/test_common.hpp"

class RawGLGemTest : public test::TestApp {
 public:
  RawGLGemTest() = default;
  ~RawGLGemTest() override = default;

 protected:
  void OnInit() override {
    glClearColor(1.f, 1.f, 1.f, 1.f);

    InitVertexBuffer();
    InitProgram();

    mvp = glm::ortho(0.f, 800.f, 600.f, 0.f);
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(program);

    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);

    if (glad_glPatchParameteri == nullptr) {
      exit(-1);
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);

    glUseProgram(0);
  }

 private:
  void InitVertexBuffer() {
    std::vector<float> raw_points{
        50.f,  300.f, 0.f, 1.f,  // p0
        300.f, 100.f, 0.f, 1.f,  // p1
        350.f, 300.f, 0.f, 1.f,  // p2
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, raw_points.size() * sizeof(float),
                 raw_points.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(0);
  }

  void InitProgram() {
    // vertex shader
    const char* vs_code = R"(
        #version 400 core


        layout(location = 0) in vec4 vPosition;
       
        void main() {
            gl_Position = vPosition;
        }
    )";

    const char* fs_code = R"(
        #version 400 core
        
        out vec4 FragColor;

        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    const char* gs_code = R"(
      #version 400 core

      layout (triangles) in;
      layout (triangle_strip, max_vertices = 96) out;

      uniform mat4 mvp;

      vec2 quad_bezier(float u, vec2 p0, vec2 p1, vec2 p2) {
            float b0 = (1.0 - u) * (1.0 - u);
            float b1 = 2.0 * u * (1.0 - u);
            float b2 = u * u;

            vec2 p = b0 * p0 + b1 * p1 + b2 * p2;
            return p;
      }

      void generate_bezier() {
        vec2 p0 = gl_in[0].gl_Position.xy;
        vec2 p1 = gl_in[1].gl_Position.xy;
        vec2 p2 = gl_in[2].gl_Position.xy;

        float step = 1.0 / 31.0;
        float u = 0.0;
        vec2 points[32];
        for(int i = 0; i < 32; i++) {
          points[i] = quad_bezier(u, p0, p1, p2);
          u += step;
        }

        for(int i = 0; i < 31; i++) {
          gl_Position = mvp * vec4(p0, 0.0, 1.0);
          EmitVertex();

          gl_Position = mvp * vec4(points[i], 0.0, 1.0);
          EmitVertex();

          gl_Position = mvp * vec4(points[i + 1], 0.0, 1.0);
          EmitVertex();

          EndPrimitive();
        }
      }

      void main() {

        generate_bezier();
      }
    )";

    const char* gs_stroke_code = R"(
      #version 400 core

      layout (triangles) in;
      layout (triangle_strip, max_vertices = 96) out;

      #define MAX_STEP 32
      #define MAX_QUAD_STEP 32
      #define STROKE_WIDTH 4
      #define StrokeWidth 10.0

      uniform mat4 mvp;

      vec2 quad_bezier(float u, vec2 p0, vec2 p1, vec2 p2) {
            float b0 = (1.0 - u) * (1.0 - u);
            float b1 = 2.0 * u * (1.0 - u);
            float b2 = u * u;

            vec2 p = b0 * p0 + b1 * p1 + b2 * p2;
            return p;
      }

      vec2 quad_bezier_tangent(float u, vec2 p0, vec2 p1, vec2 p2) {
        vec2 p = 2.0 * (1.0 - u) * (p1 - p0) + 2.0 * u * (p2 - p1);

        return normalize(p);
      }

      void generate_quad_stroke() {
        vec2 p0 = gl_in[0].gl_Position.xy;
        vec2 p1 = gl_in[1].gl_Position.xy;
        vec2 p2 = gl_in[2].gl_Position.xy;

        float step = 1.0 / float(MAX_QUAD_STEP - 1);
        float u = 0.0;
        for (int i = 0; i < MAX_QUAD_STEP; i++) {
          vec2 p = quad_bezier(u, p0, p1, p2);

          vec2 tangent = quad_bezier_tangent(u, p0, p1, p2);
          vec2 n = vec2(tangent.y, -tangent.x);

          vec2 up = p + n * StrokeWidth * 0.5;
          vec2 dp = p - n * StrokeWidth * 0.5;

          gl_Position = mvp * vec4(up, 0.0, 1.0);
          EmitVertex();

          gl_Position = mvp * vec4(dp, 0.0, 1.0);
          EmitVertex();

          u += step;
        }
        EndPrimitive();
      }

      void main() {
        generate_quad_stroke();
      }
    )";

    program = test::create_shader_program(vs_code, fs_code, gs_stroke_code);

    mvp_location = glGetUniformLocation(program, "mvp");
  }

 private:
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint count = 3;
  GLuint program = 0;
  glm::mat4 mvp = {};
  GLuint mvp_location = -1;
};

int main(int argc, const char** argv) {
  RawGLGemTest app;
  app.Start();
  return 0;
}