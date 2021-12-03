#include "vk/vk_app.hpp"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <iostream>
#include <limits>
#include <shader.hpp>
#include <vector>

namespace example {

VkApp::VkApp(int32_t width, int32_t height, std::string name)
    : width_(width), height_(height), window_name_(name) {}

VkApp::~VkApp() = default;

void VkApp::Run() {
  glfwInit();
  // no need OpenGL api
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // disable resize since recreate pipeline is not ready
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  // create window
  window_ =
      glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

  SetupVkContext();

  Loop();

  CleanUp();
}

void VkApp::SetupVkContext() { OnStart(); }

void VkApp::Loop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void VkApp::CleanUp() {
  OnDestroy();
  glfwDestroyWindow(window_);
  glfwTerminate();
}

}  // namespace example