#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

GLFWwindow* init_glfw_window(uint32_t width, uint32_t height,
                             const char* title) {
  if (!glfwInit()) {
    exit(-1);
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  return glfwCreateWindow(width, height, title, nullptr, nullptr);
}

std::vector<const char*> required_vk_instance_extension_names{
    VK_KHR_SURFACE_EXTENSION_NAME};

vk::UniqueInstance create_vulkan_instance() {
  vk::ApplicationInfo app_info{"Hello Vulkan", VK_MAKE_VERSION(1, 0, 0),
                               "Skity", VK_MAKE_VERSION(1, 0, 0),
                               VK_API_VERSION_1_2};

  std::vector<vk::ExtensionProperties> instance_extension_properties =
      vk::enumerateInstanceExtensionProperties();

  for (const auto& ep : instance_extension_properties) {
    std::cout << "extension :  " << ep.extensionName << std::endl;
  }

  std::vector<vk::LayerProperties> instance_layer_properties =
      vk::enumerateInstanceLayerProperties();

  std::vector<const char*> instance_layer_names = {};
  std::vector<const char*> instance_extension_names = {
      required_vk_instance_extension_names};

  vk::InstanceCreateInfo instance_create_info{
      {}, &app_info, instance_layer_names, instance_extension_names};

  return vk::createInstanceUnique(instance_create_info);
}

int main(int argc, const char** argv) {
  GLFWwindow* window = init_glfw_window(800, 800, "Hello Vulkan");
  if (!window) {
    exit(-1);
  }

  auto vk_instance = create_vulkan_instance();

  if (!vk_instance) {
    exit(-1);
  }

  auto vk_physical_devices = vk_instance->enumeratePhysicalDevices();

  for (const auto& vk_pd : vk_physical_devices) {
    std::cout << "device : " << vk_pd.getProperties().deviceName << std::endl;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
