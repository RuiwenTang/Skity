#include <glad/glad.h>
// should after glad.h
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <skity/render/canvas.hpp>
#include <skity/svg/svg_dom.hpp>

#include "example_config.hpp"

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

std::unique_ptr<skity::SVGDom> init_simple_svg() {
  static std::string simple_svg = R"(
<svg width="200px" height="100px" viewBox="0 0 95 50"
     xmlns="http://www.w3.org/2000/svg">
  <g id="g12" stroke="green" fill="#F00" stroke-width="5">
    <circle cx="25" cy="25" r="15" />
    <circle cx="40" cy="25" r="15" />
    <circle cx="55" cy="25" r="15" />
    <circle cx="70" cy="25" r="15" />
 <path d="M 100 100 L 300 100 L 200 300 z"
        fill="red" stroke="blue" stroke-width="3" />
  </g>
</svg>
)";

  //  auto dom = skity::SVGDom::MakeFromString(simple_svg);
  auto dom = skity::SVGDom::MakeFromFile(EXAMPLE_IMAGE_ROOT "/tiger.svg");
  return dom;
}

int main(int argc, const char** argv) {
  GLFWwindow* window = init_glfw_window(800, 800, "SKITY render example");

  glClearColor(0.3f, 0.4f, 0.5f, 1.f);
  glClearStencil(0x0);
  glStencilMask(0xFF);
  glEnable(GL_STENCIL_TEST);
  // blend is need for anti-alias
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  auto simple_svg = init_simple_svg();

  // auto canvas =
  //     skity::Canvas::MakeGLCanvas2(0, 0, 1000, 1000,
  //     (void*)glfwGetProcAddress);
  auto canvas = skity::Canvas::MakeHardwareAccelationCanvas(
      1000, 1000, (void*)glfwGetProcAddress);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    canvas->save();
    canvas->translate(50, 100);
    simple_svg->Render(canvas.get());
    canvas->restore();

    canvas->flush();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}