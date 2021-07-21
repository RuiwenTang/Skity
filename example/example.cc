#include <glad/glad.h>
// should after glad.h
#include <GLFW/glfw3.h>

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

GLFWwindow* init_glfw_window(uint32_t width, uint32_t height,
                             const char* title) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // multisample
  glfwWindowHint(GLFW_SAMPLES, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (window == nullptr) {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    exit(-3);
  }

  return window;
}

int main(int argc, const char** argv) {
  GLFWwindow* window = init_glfw_window(800, 600, "SKITY render example");

  glClearColor(0.3f, 0.4f, 0.5f, 1.f);
  glClearStencil(0x0);
  glStencilMask(0xFF);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}