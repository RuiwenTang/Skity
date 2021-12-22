#ifndef EXAMPLE_UTILS_GL_APP_HPP
#define EXAMPLE_UTILS_GL_APP_HPP

#include <glad/glad.h>
// this should after glad
#include <GLFW/glfw3.h>

#include <skity/skity.hpp>
#include <string>

namespace example {

class GLApp {
 public:
  GLApp(int32_t width, int32_t height, std::string name,
        glm::vec4 const& clear_color = {0.3f, 0.4f, 0.5f, 1.f});
  virtual ~GLApp();

  void Run();

  int32_t ScreenWidth() const { return width_; }
  int32_t ScreenHeight() const { return height_; }

 protected:
  virtual void OnStart() {}
  virtual void OnUpdate(float elapsed_time) {}
  virtual void OnDestroy() {}

  void GetCursorPos(double& x, double& y);

  skity::Canvas* GetCanvas() const { return canvas_.get(); }

 private:
  void Init();
  void RunLoop();
  void Destroy();

 private:
  int32_t width_ = {};
  int32_t height_ = {};
  std::string name_ = {};
  GLFWwindow* window_ = {};
  glm::vec4 clear_color_ = {};
  std::unique_ptr<skity::Canvas> canvas_ = {};
};

}  // namespace example

#endif  // EXAMPLE_UTILS_GL_APP_HPP