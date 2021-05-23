#include <glad/glad.h>
// glad must before glw3.h
#include <GLFW/glfw3.h>

namespace test {
GLFWwindow* init_glfw_window(uint32_t width, uint32_t height);

GLuint create_shader_program(const char* vs_code, const char* fs_code);
}  // namespace test