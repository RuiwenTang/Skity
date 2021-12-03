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

void VkApp::Run() {}

}  // namespace example