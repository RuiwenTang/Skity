#include "vk/vk_app.hpp"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <shader.hpp>
#include <vector>

#include "spdlog/spdlog.h"

namespace example {

#ifndef SKITY_RELEASE
static bool g_enable_validation = true;
#else
static bool g_enable_validation = false;
#endif

#ifndef SKITY_RELEASE

static VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

static void destroy_debug_utils_messenger_ext(
    VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* p_allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func) {
    func(instance, debug_messenger, p_allocator);
  }
}

#endif

static const char* g_validation_name = "VK_LAYER_KHRONOS_validation";

static bool check_validation_layer_support() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> layers{layer_count};
  vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

  for (auto const& property : layers) {
    if (std::strcmp(property.layerName, g_validation_name) == 0) {
      return true;
    }
  }

  return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT messageType,
               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
               void* pUserData) {
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    spdlog::error("{} Code {} : {}", pCallbackData->pMessageIdName,
                  pCallbackData->messageIdNumber, pCallbackData->pMessage);
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    spdlog::warn("{} Code {} : {}", pCallbackData->pMessageIdName,
                 pCallbackData->messageIdNumber, pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    spdlog::critical("{} Code {} : {}", pCallbackData->pMessageIdName,
                     pCallbackData->messageIdNumber, pCallbackData->pMessage);
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    spdlog::trace("{} Code {} : {}", pCallbackData->pMessageIdName,
                  pCallbackData->messageIdNumber, pCallbackData->pMessage);
  } else {
    spdlog::info("{} Code {} : {}", pCallbackData->pMessageIdName,
                 pCallbackData->messageIdNumber, pCallbackData->pMessage);
  }

  return VK_FALSE;
}

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

  // init log property
  spdlog::flush_every(std::chrono::seconds(1));

  SetupVkContext();

  Loop();

  CleanUp();
}

void VkApp::SetupVkContext() {
  CreateVkInstance();

  OnStart();
}

void VkApp::Loop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void VkApp::CleanUp() {
  OnDestroy();

  vkDestroyInstance(vk_instance_, nullptr);

  glfwDestroyWindow(window_);
  glfwTerminate();
}

// vulkan util function
void VkApp::CreateVkInstance() {
  if (g_enable_validation && !check_validation_layer_support()) {
    spdlog::error("validation layers requested but not available!");
    exit(-1);
  }

  VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app_info.pApplicationName = window_name_.c_str();
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Skity";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  create_info.pApplicationInfo = &app_info;

  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extension_names{glfw_extension_count};
  for (uint32_t i = 0; i < glfw_extension_count; i++) {
    extension_names[i] = glfw_extensions[i];
  }

  if (g_enable_validation) {
    extension_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  create_info.enabledExtensionCount = (uint32_t)extension_names.size();
  create_info.ppEnabledExtensionNames = extension_names.data();

  if (g_enable_validation) {
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debug_callback;

    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = &g_validation_name;
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
  } else {
    create_info.enabledLayerCount = 0;
  }
  VkResult ret = vkCreateInstance(&create_info, nullptr, &vk_instance_);
  if (ret != VK_SUCCESS) {
    spdlog::error("Failed to create Vulkan instance");
    exit(-1);
  }

  if (g_enable_validation) {
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debug_callback;

    if (create_debug_utils_messenger_ext(vk_instance_, &debug_create_info,
                                         nullptr,
                                         &vk_debug_messenger_) != VK_SUCCESS) {
      spdlog::error("Failed to set up debug messenger!");
      exit(-1);
    }
  }

  spdlog::info("Create instance success");
}

}  // namespace example