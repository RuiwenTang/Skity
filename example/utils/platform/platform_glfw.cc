#include "utils/platform/platform_glfw.hpp"

#include "utils/vk_app.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <vulkan/vulkan.hpp>

namespace example {

static void glfw_frame_buffer_resize_callback(GLFWwindow* window, int32_t width,

                                              int32_t height) {}

void PlatformGLFW::StartUp() {
  if (!glfwInit()) {
    exit(-1);
  }
}

void PlatformGLFW::CleanUp() { glfwTerminate(); }

void* PlatformGLFW::CreateWindow(uint32_t window_width, uint32_t window_height,
                                 std::string const& title) {
  // no need glfw to init OpenGL context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                        title.c_str(), nullptr, nullptr);

  if (!window) {
    exit(-1);
  }
  return window;
}

void PlatformGLFW::StartEventLoop(VkApp* app) {
  auto window = reinterpret_cast<GLFWwindow*>(app->GetWindowHandle());

  glfwSetWindowUserPointer(window, app);
  glfwSetFramebufferSizeCallback(window, &glfw_frame_buffer_resize_callback);

  auto tp1 = std::chrono::system_clock::now();
  auto tp2 = std::chrono::system_clock::now();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    tp2 = std::chrono::system_clock::now();
    auto elapsed_time = tp2 - tp1;
    tp1 = tp2;

    float elapsed_time_value = elapsed_time.count();

    app->Update(elapsed_time_value);
  }
}

void PlatformGLFW::DestroyWindow(void* window) {
  glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(window));
}

Platform::WindowSize PlatformGLFW::GetWindowSize(void* window) {
  WindowSize size{};

  GLFWwindow* glfwWindow = reinterpret_cast<GLFWwindow*>(window);
  glfwGetFramebufferSize(glfwWindow, &size[0], &size[1]);

  return size;
}
std::vector<const char*> PlatformGLFW::GetRequiredInstanceExtensions() {
  return std::vector<const char*>{VK_KHR_SURFACE_EXTENSION_NAME};
}

}  // namespace example
