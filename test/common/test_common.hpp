#pragma once

#include <glad/glad.h>
// glad must before glw3.h
#include <GLFW/glfw3.h>

namespace test {
GLFWwindow* init_glfw_window(uint32_t width, uint32_t height);

GLuint create_shader_program(const char* vs_code, const char* fs_code);

GLuint create_shader_program(const char* vs_code, const char* fs_code,
                             const char* gs_code);

GLuint create_shader_program(const char* vs_code, const char* fs_code,
                             const char* tcs_code, const char* tes_code);

class TestApp {
 public:
  TestApp(uint32_t width = 800, uint32_t height = 600)
      : window_width_(width), window_height_(height), window_(nullptr) {}

  virtual ~TestApp();

  void Start();
  void UpdateWindowSize(uint32_t width, uint32_t height);

 protected:
  virtual void OnInit() = 0;
  virtual void OnDraw() = 0;
  virtual void OnWindowSizeUpdate(uint32_t width, uint32_t height) {}
  float Density() { return density_; }

 private:
  void Init();
  void RunLoop();

 private:
  uint32_t window_width_;
  uint32_t window_height_;
  float density_ = 1.f;
  GLFWwindow* window_;
};

}  // namespace test