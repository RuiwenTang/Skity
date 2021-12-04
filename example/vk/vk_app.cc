#include "vk/vk_app.hpp"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <set>
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
  auto logger = spdlog::default_logger();
  auto level = spdlog::level::debug;

  if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    level = spdlog::level::err;
  } else if (messageSeverity ==
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    level = spdlog::level::warn;
  }

  logger->log(level, "{} Code {} : {}", pCallbackData->pMessageIdName,
              pCallbackData->messageIdNumber, pCallbackData->pMessage);
  if (0 < pCallbackData->queueLabelCount) {
    logger->log(level, "\t Queue Labels:");
    for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
      logger->log(level, "\t\t labelName = [ {} ]",
                  pCallbackData->pQueueLabels[i].pLabelName);
    }
  }

  if (0 < pCallbackData->cmdBufLabelCount) {
    logger->log(level, "\t CMD labels: \n");
    for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
      logger->log(level, "\t\t labelName = [ {} ]",
                  pCallbackData->pCmdBufLabels[i].pLabelName);
    }
  }

  if (0 < pCallbackData->objectCount) {
    for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
      logger->log(
          level,
          "\t Object [{}]  \t\t objectType <{}> handle [{:X}] name : <{}>",
          i + 1, pCallbackData->pObjects[i].objectType,
          pCallbackData->pObjects[i].objectHandle,
          pCallbackData->pObjects[i].pObjectName);
    }
  }

  return VK_FALSE;
}

static VkFormat choose_swap_chain_format(VkPhysicalDevice phy_device,
                                         VkSurfaceKHR surface) {
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, surface, &format_count,
                                       nullptr);

  if (format_count == 0) {
    return VK_FORMAT_UNDEFINED;
  }

  std::vector<VkSurfaceFormatKHR> formats{format_count};
  vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, surface, &format_count,
                                       formats.data());

  if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    return VK_FORMAT_R8G8B8A8_UNORM;
  }

  for (VkSurfaceFormatKHR format : formats) {
    if (format.format == VK_FORMAT_R8G8B8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format.format;
    }
  }

  return formats[0].format;
}

VkApp::VkApp(int32_t width, int32_t height, std::string name)
    : width_(width), height_(height), window_name_(name) {}

VkApp::~VkApp() = default;

void VkApp::Run() {
  if (g_enable_validation) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

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

void VkApp::SetupVkContext() {
  CreateVkInstance();
  CreateVkSurface();
  PickPhysicalDevice();
  CreateVkDevice();
  CreateSwapChain();
  CreateSwapChainImageViews();

  OnStart();
}

void VkApp::Loop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  
    
  }

  vkDeviceWaitIdle(vk_device_);
}

void VkApp::CleanUp() {
  OnDestroy();

  for (auto image_view : swap_chain_image_views) {
    vkDestroyImageView(vk_device_, image_view, nullptr);
  }
  vkDestroySwapchainKHR(vk_device_, vk_swap_chain_, nullptr);
  vkDestroyDevice(vk_device_, nullptr);
  vkDestroySurfaceKHR(vk_instance_, vk_surface_, nullptr);
  destroy_debug_utils_messenger_ext(vk_instance_, vk_debug_messenger_, nullptr);
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
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = &g_validation_name;
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

void VkApp::CreateVkSurface() {
  if (glfwCreateWindowSurface(vk_instance_, window_, nullptr, &vk_surface_) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create window surface!");
    exit(-1);
  }
}

void VkApp::PickPhysicalDevice() {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(vk_instance_, &device_count, nullptr);

  if (device_count == 0) {
    spdlog::error("Failed to find GPU support vulkan");
    exit(-1);
  }

  std::vector<VkPhysicalDevice> available_devices{device_count};
  vkEnumeratePhysicalDevices(vk_instance_, &device_count,
                             available_devices.data());

  int32_t graphic_queue_family = -1;
  int32_t present_queue_family = -1;

  for (size_t i = 0; i < available_devices.size(); i++) {
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(available_devices[i], &queue_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties{queue_count};
    vkGetPhysicalDeviceQueueFamilyProperties(available_devices[i], &queue_count,
                                             queue_family_properties.data());

    auto graphic_it = std::find_if(
        queue_family_properties.begin(), queue_family_properties.end(),
        [](VkQueueFamilyProperties props) {
          return props.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        });

    auto present_it = std::find_if(
        queue_family_properties.begin(), queue_family_properties.end(),
        [](VkQueueFamilyProperties props) {
          return props.queueFlags & VK_QUEUE_PROTECTED_BIT;
        });
    if (graphic_it != queue_family_properties.end()) {
      vk_phy_device_ = available_devices[i];
      graphic_queue_family =
          std::distance(graphic_it, queue_family_properties.begin());

      present_queue_family =
          std::distance(present_it, queue_family_properties.begin());
      break;
    }
  }

  if (graphic_queue_family == -1 || present_queue_family == -1) {
    spdlog::error("Can not find GPU contains Graphic support");
    exit(-1);
  }

  VkBool32 support = 0;
  vkGetPhysicalDeviceSurfaceSupportKHR(vk_phy_device_, graphic_queue_family,
                                       vk_surface_, &support);
  if (support == VK_TRUE) {
    present_queue_family = graphic_queue_family;
  }

  graphic_queue_index_ = graphic_queue_family;
  present_queue_index_ = present_queue_family;
}

void VkApp::CreateVkDevice() {
  std::vector<VkDeviceQueueCreateInfo> queue_create_info{};

  std::set<uint32_t> queue_families = {graphic_queue_index_,
                                       present_queue_index_};
  float queue_priority = 1.f;

  for (uint32_t family : queue_families) {
    VkDeviceQueueCreateInfo create_info{
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    create_info.queueFamilyIndex = family;
    create_info.queueCount = 1;
    create_info.pQueuePriorities = &queue_priority;

    queue_create_info.emplace_back(create_info);
  }

  VkPhysicalDeviceFeatures device_features{};

  std::vector<const char*> required_device_extension{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(vk_phy_device_, nullptr, &count,
                                         nullptr);

    std::vector<VkExtensionProperties> properties(count);
    vkEnumerateDeviceExtensionProperties(vk_phy_device_, nullptr, &count,
                                         properties.data());

    auto it = std::find_if(
        properties.begin(), properties.end(), [](VkExtensionProperties prop) {
          return std::strcmp(prop.extensionName, "VK_KHR_portability_subset") ==
                 0;
        });

    if (it != properties.end()) {
      // VUID-VkDeviceCreateInfo-pProperties-04451
      required_device_extension.emplace_back("VK_KHR_portability_subset");
    }
  }

  VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  create_info.pQueueCreateInfos = queue_create_info.data();
  create_info.queueCreateInfoCount = queue_create_info.size();
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = required_device_extension.size();
  create_info.ppEnabledExtensionNames = required_device_extension.data();

  if (vkCreateDevice(vk_phy_device_, &create_info, nullptr, &vk_device_) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create logical device");
    exit(-1);
  }

  vkGetDeviceQueue(vk_device_, graphic_queue_index_, 0, &vk_graphic_queue_);
  vkGetDeviceQueue(vk_device_, present_queue_index_, 0, &vk_present_queue_);
}

void VkApp::CreateSwapChain() {
  VkFormat format = choose_swap_chain_format(vk_phy_device_, vk_surface_);

  VkSurfaceCapabilitiesKHR surface_caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_phy_device_, vk_surface_,
                                            &surface_caps);

  VkCompositeAlphaFlagBitsKHR surface_composite;
  if (surface_caps.supportedCompositeAlpha &
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
    surface_composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  } else if (surface_caps.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
    surface_composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
  } else if (surface_caps.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
    surface_composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
  } else {
    surface_composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  }

  VkSwapchainCreateInfoKHR create_info{
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  create_info.surface = vk_surface_;
  create_info.minImageCount = std::max(uint32_t(2), surface_caps.minImageCount);
  create_info.imageFormat = format;
  create_info.imageExtent = surface_caps.currentExtent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  create_info.queueFamilyIndexCount = 1;
  create_info.pQueueFamilyIndices = &present_queue_index_;
  create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  create_info.compositeAlpha = surface_composite;
  create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  create_info.oldSwapchain = nullptr;

  if (vkCreateSwapchainKHR(vk_device_, &create_info, nullptr,
                           &vk_swap_chain_) != VK_SUCCESS) {
    spdlog::error("Failed to create swap chain.");
    exit(-1);
  }

  swap_chain_format_ = format;
  swap_chain_extend_ = surface_caps.currentExtent;
}

void VkApp::CreateSwapChainImageViews() {
  uint32_t image_count = 0;
  vkGetSwapchainImagesKHR(vk_device_, vk_swap_chain_, &image_count, nullptr);

  std::vector<VkImage> swap_chain_image{image_count};
  vkGetSwapchainImagesKHR(vk_device_, vk_swap_chain_, &image_count,
                          swap_chain_image.data());

  swap_chain_image_views.resize(image_count);
  for (uint32_t i = 0; i < image_count; i++) {
    VkImageViewCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.image = swap_chain_image[i];
    create_info.format = swap_chain_format_;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (vkCreateImageView(vk_device_, &create_info, nullptr,
                          &swap_chain_image_views[i]) != VK_SUCCESS) {
      spdlog::error("Failed to create swap chain image view");
      exit(-1);
    }
  }
}

}  // namespace example
