
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <string>

#include "common/test_common.hpp"
#include "src/render/path_raster.hpp"
#include "src/render/stroke.hpp"

using namespace skity;

struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint front = 0;
  GLuint back = 0;
};

Path make_stroke() {
  Path path;

  path.moveTo(40, 40);
  path.lineTo(200, 200);
  path.lineTo(300, 50);

  path.moveTo(10, 10);
  path.quadTo(256, 64, 128, 128);
  path.quadTo(10, 192, 250, 250);
  path.addOval(Rect::MakeLTRB(100, 100, 300, 500));
  path.addCircle(200, 200, 100);
  path.addOval(Rect::MakeLTRB(50, 200, 300, 500));

  Paint paint;
  paint.setStrokeWidth(10.f);
  paint.setStrokeCap(Paint::kRound_Cap);
  paint.setStrokeJoin(Paint::kRound_Join);

  Path dst;
  Stroke stroke(paint);

  stroke.strokePath(path, &dst);
  return dst;
}

Path make_fill() {
  Paint paint;
  paint.setStrokeWidth(10.f);
  paint.setStrokeCap(Paint::kRound_Cap);
  paint.setStrokeJoin(Paint::kRound_Join);

  Path path;
  path.moveTo(10, 10);
  path.quadTo(256, 64, 128, 128);
  path.quadTo(10, 192, 250, 250);
  path.close();
  return path;
}

std::unique_ptr<PathVertex> raster_path(Path const& path) {
  PathRaster raster;

  return raster.rasterPath(path);
}

GLuint prepare_shader() {
  const char* vs = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 mvp;

    void main() {
      gl_Position = mvp * vec4(aPos, 1.0);
    }
  )";

  const char* fs = R"(
    #version 330 core

    out vec4 FragColor;
    uniform vec4 color;

    void main() {
      FragColor = color;
    }
  )";

  return test::create_shader_program(vs, fs);
}

Mesh setup_mesh(PathVertex* vertex) {
  Mesh mesh;

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(3, &mesh.vbo);

  glBindVertexArray(mesh.vao);

  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertex->vertices().size() * sizeof(Point),
               vertex->vertices().data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.front);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vertex->frontVerticesIndex().size() * sizeof(uint32_t),
               vertex->frontVerticesIndex().data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.back);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vertex->backVerticesIndex().size() * sizeof(uint32_t),
               vertex->backVerticesIndex().data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return mesh;
}

void render_window(GLFWwindow* window, PathVertex* path_vertex) {
  path_vertex->dump();
  glClearColor(0.3, 0.4, 0.5, 1.0);
  glClearStencil(0x10);
  glStencilMask(0xFF);
  glEnable(GL_STENCIL_TEST);
  glDisable(GL_DEPTH_TEST);

  GLuint shader = prepare_shader();
  GLint mvp_location = glGetUniformLocation(shader, "mvp");
  GLint color_location = glGetUniformLocation(shader, "color");
  Mesh mesh = setup_mesh(path_vertex);
  glm::mat4 mvp = glm::ortho<float>(0, 800, 600, 0, -100, 100);

  glUseProgram(shader);

  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
  while (!glfwWindowShouldClose(window)) {
    glEnable(GL_STENCIL_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glStencilMask(0x0F);
    glUseProgram(shader);
    glUniform4f(color_location, 1.f, 1.f, 0.f, 1.f);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // step 1 disable color and stencil path
    glColorMask(0, 0, 0, 0);
    glStencilFunc(GL_ALWAYS, 0x01, 0x0F);

    // front triangles
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.front);
    glDrawElements(GL_TRIANGLES, path_vertex->frontVerticesIndex().size(),
                   GL_UNSIGNED_INT, (void*)0);
    // back triangles
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.back);
    glDrawElements(GL_TRIANGLES, path_vertex->backVerticesIndex().size(),
                   GL_UNSIGNED_INT, (void*)0);

    // step 2 enable color and stencil test fill
    glColorMask(1, 1, 1, 1);
    glStencilMask(0x1F);
    //    glStencilFunc(GL_EQUAL, 0x00, 0x1F);
    //    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    //
    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.front);
    //    glDrawElements(GL_TRIANGLES, path_vertex->frontVerticesIndex().size(),
    //                   GL_UNSIGNED_INT, (void*)0);

    glUniform4f(color_location, 1.f, 0.f, 0.f, 1.f);

    glStencilFunc(GL_NOTEQUAL, 0x10, 0x1F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.front);
    glDrawElements(GL_TRIANGLES, path_vertex->frontVerticesIndex().size(),
                   GL_UNSIGNED_INT, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.back);
    glDrawElements(GL_TRIANGLES, path_vertex->backVerticesIndex().size(),
                   GL_UNSIGNED_INT, (void*)0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

int main(int argc, const char** argv) {
  Path path = make_stroke();
  path.dump();
  auto rastered_path = raster_path(path);

  GLFWwindow* window = test::init_glfw_window(800, 600);

  render_window(window, rastered_path.get());
  return 0;
}
