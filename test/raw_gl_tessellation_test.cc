#include <vector>

#include "test/common/test_common.hpp"

class RawGLGemTest : public test::TestApp {
 public:
  RawGLGemTest() = default;
  ~RawGLGemTest() override = default;

 protected:
  void OnInit() override {
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClearStencil(0x0);
    glEnable(GL_STENCIL_TEST);

    InitVertexBuffer();
    InitProgram();
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(program);

    if (glad_glPatchParameteri == nullptr) {
      exit(-1);
    }

    glColorMask(0, 0, 0, 0);
    glStencilFunc(GL_ALWAYS, 0x01, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glDrawArrays(GL_PATCHES, 3, 3);

    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    glDrawArrays(GL_PATCHES, 0, 3);

    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glDrawArrays(GL_PATCHES, 3, 3);

    glBindVertexArray(0);

    glUseProgram(0);
  }

 private:
  void InitVertexBuffer() {
    std::vector<float> raw_points{
        -0.5f, -0.5f, 1.f, 1.f,  // p0
        0.3f,  0.5f,  1.f, 1.f,  // p1
        0.5f,  -0.3f, 1.f, 1.f,  // p2
        -0.5f, -0.5f, 0.f, 1.f,  // p0
        0.3f,  0.5f,  0.f, 1.f,  // p1
        0.5f,  -0.3f, 0.f, 1.f,  // p2
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

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

    const char* tcs_code = R"(
        #version 400 core
        
        layout(vertices = 3) out;
        
        void main(){
            gl_TessLevelInner[0] = 1;
            gl_TessLevelOuter[0] = 1;
            gl_TessLevelOuter[1] = 1;
            if (gl_in[0].gl_Position.z == 0.0) {
              gl_TessLevelOuter[2] = 1;
            } else {
              gl_TessLevelOuter[2] = 32;
            }
        }
    )";

    const char* tes_code = R"(
        #version 400 core
        
        layout(triangles) in;

        vec2 quad_bezier(float u, vec2 p0, vec2 p1, vec2 p2) {
            float b0 = (1.0 - u) * (1.0 - u);
            float b1 = 2.0 * u * (1.0 - u);
            float b2 = u * u;

            vec2 p = b0 * p0 + b1 * p1 + b2 * p2;
            return p;
        }

        void main() {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float w = gl_TessCoord.z;

            if (gl_in[0].gl_Position.z == 0) {
              gl_Position = (u * gl_in[0].gl_Position) + (v * gl_in[1].gl_Position) + (w * gl_in[2].gl_Position);
            } else {
              vec2 p = quad_bezier(u, vec2(gl_in[0].gl_Position), vec2(gl_in[1].gl_Position), vec2(gl_in[2].gl_Position));
            
              gl_Position = vec4(p, 0.0, 1.0) + (w * gl_in[2].gl_Position);
            }
        }
    )";

    program = test::create_shader_program(vs_code, fs_code, tcs_code, tes_code);
  }

 private:
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint count = 3;
  GLuint program = 0;
};

int main(int argc, const char** argv) {
  RawGLGemTest app;
  app.Start();
  return 0;
}
