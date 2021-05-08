#include <glad/glad.h>
// glad must before glw3.h
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <string>

#include "src/render/path_raster.hpp"
#include "src/render/stroke.hpp"

using namespace skity;

struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint front = 0;
  GLuint back = 0;
};

Path make_path()
{
  Path path;

  path.moveTo(40, 40);
  path.lineTo(200, 200);
  path.lineTo(300, 100);

  // path.addOval(Rect::MakeLTRB(100, 100, 500, 500));

  Paint paint;
  paint.setStrokeWidth(10.f);
  paint.setStrokeCap(Paint::kButt_Cap);
  paint.setStrokeJoin(Paint::kRound_Join);

  Path dst;
  Stroke stroke(paint);

  stroke.strokePath(path, &dst);
  return dst;
}

std::unique_ptr<PathVertex> raster_path(Path const& path)
{
  PathRaster raster;

  return raster.rasterPath(path);
}

GLFWwindow* init_glfw_window(int width, int height)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window =
      glfwCreateWindow(width, height, "Raw path render", nullptr, nullptr);

  if (window == nullptr) {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    exit(-3);
  }

  return window;
}

GLuint prepare_shader()
{
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

  GLint success;
  // vs
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vs, nullptr);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    exit(-4);
  }
  // fs
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fs, nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    exit(-4);
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (!success) {
    exit(-5);
  }
  return program;
}

Mesh setup_mesh(PathVertex* vertex)
{
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

void render_window(GLFWwindow* window, PathVertex* path_vertex)
{
  path_vertex->dump();
  glClearColor(0.3, 0.4, 0.5, 1.0);
  glClearStencil(0);
  glStencilMask(0xFF);
  glEnable(GL_STENCIL_TEST);

  GLuint shader = prepare_shader();
  GLint mvp_location = glGetUniformLocation(shader, "mvp");
  GLint color_location = glGetUniformLocation(shader, "color");
  Mesh mesh = setup_mesh(path_vertex);
  glm::mat4 mvp = glm::ortho<float>(0, 800, 600, 0, -100, 100);

  glUseProgram(shader);
  glUniform4f(color_location, 1.f, 1.f, 0.f, 1.f);
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader);

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

    glStencilFunc(GL_ALWAYS, 0x00, 0x0F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.back);
    glDrawElements(GL_TRIANGLES, path_vertex->backVerticesIndex().size(),
                   GL_UNSIGNED_INT, (void*)0);

    // step 2 enable color and stencil test fill
    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_EQUAL, 0x01, 0x1F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.front);
    glDrawElements(GL_TRIANGLES, path_vertex->frontVerticesIndex().size(),
                   GL_UNSIGNED_INT, (void*)0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

int main(int argc, const char** argv)
{
  Path path = make_path();

  auto rastered_path = raster_path(path);

  GLFWwindow* window = init_glfw_window(800, 600);

  render_window(window, rastered_path.get());
  return 0;
}
