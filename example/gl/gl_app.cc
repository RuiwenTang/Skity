#include "gl/gl_app.hpp"

#include <chrono>

#include "example_config.hpp"

namespace example {

GLFWwindow* init_glfw_window(uint32_t width, uint32_t height,
                             const char* title) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // multisample
  glfwWindowHint(GLFW_SAMPLES, 8);
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

GLApp::GLApp(int32_t width, int32_t height, std::string name,
             const glm::vec4& clear_color)
    : width_(width), height_(height), name_(name), clear_color_(clear_color) {}

GLApp::~GLApp() = default;

void GLApp::Run() {
  Init();
  RunLoop();
  Destroy();
}

void GLApp::Init() {
  window_ = init_glfw_window(width_, height_, name_.c_str());

  glClearColor(clear_color_.x, clear_color_.y, clear_color_.z, clear_color_.w);
  glClearStencil(0x0);
  glStencilMask(0xFF);
  // blend is need for anti-alias
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_STENCIL_TEST);

  int32_t pp_width, pp_height;
  glfwGetFramebufferSize(window_, &pp_width, &pp_height);

  float density = (float)(pp_width * pp_width + pp_height * pp_height) /
                  (float)(width_ * width_ + height_ * height_);

  auto ctx = std::make_shared<skity::GPUContext>(skity::GPUBackendType::kOpenGL,
                                                 (void*)glfwGetProcAddress);
  canvas_ = skity::Canvas::MakeHardwareAccelationCanvas(width_, height_,
                                                        density, ctx);

  canvas_->setDefaultTypeface(
      skity::Typeface::MakeFromFile(EXAMPLE_DEFAULT_FONT));
  OnStart();
}

void GLApp::RunLoop() {
  float frame_count = 0;
  uint64_t frame_total_caust = 0;
  while (!glfwWindowShouldClose(window_)) {
    auto frame_start = std::chrono::system_clock::now();
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    this->OnUpdate(0.f);
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}

void GLApp::Destroy() {
  OnDestroy();

  canvas_.reset();
}

void GLApp::GetCursorPos(double& x, double& y) {
  double mx, my;
  glfwGetCursorPos(window_, &mx, &my);

  x = mx;
  y = my;
}

}  // namespace example