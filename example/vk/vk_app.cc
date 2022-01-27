#include "vk/vk_app.hpp"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <set>
#include <vector>

#include "example_config.hpp"
#include "spdlog/spdlog.h"

namespace example {

#define ENABLE_VALIDATION

#ifdef ENABLE_VALIDATION
static bool g_enable_validation = true;
#else
static bool g_enable_validation = false;
#endif

#ifdef ENABLE_VALIDATION

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
          pCallbackData->pObjects[i].pObjectName
              ? pCallbackData->pObjects[i].pObjectName
              : "unknown");
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

static bool get_support_depth_format(VkPhysicalDevice phy_devce,
                                     VkFormat* out_format) {
  // Since all depth formats may be optional, we need to find a suitable depth
  // format to use
  // Start with the highest precision packed format
  std::vector<VkFormat> depthFormats = {
      VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
      VK_FORMAT_D16_UNORM};

  for (auto& format : depthFormats) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(phy_devce, format, &formatProps);
    // Format must support depth stencil attachment for optimal tiling
    if (formatProps.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      *out_format = format;
      return true;
    }
  }

  return false;
}

VkSampleCountFlagBits get_max_usable_sample_count(
    VkPhysicalDeviceProperties props) {
  VkSampleCountFlags counts = props.limits.framebufferColorSampleCounts &
                              props.limits.framebufferStencilSampleCounts;

  if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

VkApp::VkApp(int32_t width, int32_t height, std::string name,
             glm::vec4 const& clear_color)
    : skity::GPUVkContext((void*)vkGetDeviceProcAddr),
      width_(width),
      height_(height),
      window_name_(name),
      clear_color_(clear_color) {}

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

  int32_t pp_width, pp_height;
  glfwGetFramebufferSize(window_, &pp_width, &pp_height);

  density_ = (float)(pp_width * pp_width + pp_height * pp_height) /
             (float)(width_ * width_ + height_ * height_);

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
  CreateCommandPool();
  CreateCommandBuffers();
  CreateSyncObjects();
  CreateRenderPass();
  CreateFramebuffers();

  OnStart();
}

void VkApp::OnStart() {
  canvas_ = skity::Canvas::MakeHardwareAccelationCanvas(width_, height_,
                                                        ScreenDensity(), this);

  canvas_->setDefaultTypeface(
      skity::Typeface::MakeFromFile(EXAMPLE_DEFAULT_FONT));
}

void VkApp::OnDestroy() { canvas_.reset(); }

void VkApp::Loop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();

    VkResult result = vkAcquireNextImageKHR(
        vk_device_, vk_swap_chain_, std::numeric_limits<uint64_t>::max(),
        present_semaphore_[frame_index_], nullptr, &current_frame_);

    assert(frame_index_ == current_frame_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      spdlog::error("need to handle window resize of recreate swap chain!");
      break;
    }

    VkCommandBuffer current_cmd = cmd_buffers_[frame_index_];
    vkResetCommandBuffer(current_cmd, 0);

    VkCommandBufferBeginInfo cmd_begin_info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(current_cmd, &cmd_begin_info) != VK_SUCCESS) {
      spdlog::error("Failed to begin cmd buffer at index : {}", current_frame_);
      break;
    }

    std::vector<VkClearValue> clear_values{3};
    clear_values[0].color = {clear_color_.x, clear_color_.y, clear_color_.z,
                             clear_color_.w};
    clear_values[1].depthStencil = {0.f, 0};
    clear_values[2].color = {clear_color_.x, clear_color_.y, clear_color_.z,
                             clear_color_.w};

    VkRenderPassBeginInfo render_pass_begin_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    render_pass_begin_info.renderPass = render_pass_;
    render_pass_begin_info.framebuffer =
        swap_chain_frame_buffers_[current_frame_];
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = swap_chain_extend_;
    render_pass_begin_info.clearValueCount = clear_values.size();
    render_pass_begin_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(current_cmd, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    OnUpdate(0.f);

    vkCmdEndRenderPass(current_cmd);

    if (vkEndCommandBuffer(current_cmd) != VK_SUCCESS) {
      spdlog::error("Failed to end cmd buffer at index : {}", frame_index_);
      break;
    }
    VkPipelineStageFlags submit_pipeline_stages =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.pWaitDstStageMask = &submit_pipeline_stages;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &present_semaphore_[frame_index_];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_semaphore_[frame_index_];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &current_cmd;

    vkResetFences(vk_device_, 1, &cmd_fences_[frame_index_]);

    if (vkQueueSubmit(vk_present_queue_, 1, &submit_info,
                      cmd_fences_[frame_index_]) != VK_SUCCESS) {
      spdlog::error("Failed to submit command buffer!");
      break;
    }

    if (vkWaitForFences(vk_device_, 1, &cmd_fences_[frame_index_], VK_TRUE,
                        std::numeric_limits<uint64_t>::max()) != VK_SUCCESS) {
      spdlog::error("Error in wait fences");
      break;
    }

    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swap_chain_;
    present_info.pImageIndices = &current_frame_;
    present_info.pWaitSemaphores = &render_semaphore_[frame_index_];
    present_info.waitSemaphoreCount = 1;

    result = vkQueuePresentKHR(vk_present_queue_, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      spdlog::error("need to handle window resize of recreate swap chain!");
      break;
    }

    frame_index_++;
    frame_index_ = frame_index_ % swap_chain_image_views.size();
  }

  vkDeviceWaitIdle(vk_device_);
}

void VkApp::CleanUp() {
  OnDestroy();

  vkDestroyRenderPass(vk_device_, render_pass_, nullptr);
  for (auto fb : swap_chain_frame_buffers_) {
    vkDestroyFramebuffer(vk_device_, fb, nullptr);
  }

  for (auto const& st : stencil_image_) {
    vkDestroyImageView(vk_device_, st.image_view, nullptr);
    vkDestroyImage(vk_device_, st.image, nullptr);
    vkFreeMemory(vk_device_, st.memory, nullptr);
  }

  for (auto const& si : sampler_image_) {
    vkDestroyImageView(vk_device_, si.image_view, nullptr);
    vkDestroyImage(vk_device_, si.image, nullptr);
    vkFreeMemory(vk_device_, si.memory, nullptr);
  }

  for (auto semp : present_semaphore_) {
    vkDestroySemaphore(vk_device_, semp, nullptr);
  }
  for (auto semp : render_semaphore_) {
    vkDestroySemaphore(vk_device_, semp, nullptr);
  }
  for (auto fence : cmd_fences_) {
    vkDestroyFence(vk_device_, fence, nullptr);
  }
  vkResetCommandPool(vk_device_, cmd_pool_,
                     VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
  vkDestroyCommandPool(vk_device_, cmd_pool_, nullptr);
  for (auto image_view : swap_chain_image_views) {
    vkDestroyImageView(vk_device_, image_view, nullptr);
  }
  vkDestroySwapchainKHR(vk_device_, vk_swap_chain_, nullptr);
  vkDestroyDevice(vk_device_, nullptr);
  vkDestroySurfaceKHR(vk_instance_, vk_surface_, nullptr);
#ifdef ENABLE_VALIDATION
  destroy_debug_utils_messenger_ext(vk_instance_, vk_debug_messenger_, nullptr);
#endif
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

  // skity need this extension to query device properties
  extension_names.emplace_back("VK_KHR_get_physical_device_properties2");

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
        //   VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debug_callback;
#ifdef ENABLE_VALIDATION
    if (create_debug_utils_messenger_ext(vk_instance_, &debug_create_info,
                                         nullptr,
                                         &vk_debug_messenger_) != VK_SUCCESS) {
      spdlog::error("Failed to set up debug messenger!");
      exit(-1);
    }
#endif
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
  int32_t compute_queue_family = -1;

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

    auto compute_it = std::find_if(
        queue_family_properties.begin(), queue_family_properties.end(),
        [](VkQueueFamilyProperties props) {
          return props.queueFlags & VK_QUEUE_COMPUTE_BIT;
        });

    if (graphic_it != queue_family_properties.end() &&
        compute_it != queue_family_properties.end()) {
      vk_phy_device_ = available_devices[i];
      graphic_queue_family =
          std::distance(graphic_it, queue_family_properties.begin());

      present_queue_family =
          std::distance(present_it, queue_family_properties.begin());

      compute_queue_family =
          std::distance(compute_it, queue_family_properties.begin());
      break;
    }
  }

  VkPhysicalDeviceProperties phy_props;
  vkGetPhysicalDeviceProperties(vk_phy_device_, &phy_props);

  spdlog::info("Picked device name = {}", phy_props.deviceName);

  {
    // query all extensions
    uint32_t entry_count = 0;
    vkEnumerateDeviceExtensionProperties(vk_phy_device_, nullptr, &entry_count,
                                         nullptr);
    std::vector<VkExtensionProperties> entries{entry_count};
    vkEnumerateDeviceExtensionProperties(vk_phy_device_, nullptr, &entry_count,
                                         entries.data());
    for (auto ext : entries) {
      spdlog::info("ext : {}", ext.extensionName);
    }
    // query dynamic feature
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_states{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
    VkPhysicalDeviceFeatures2 phy_features2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

    phy_features2.pNext = &dynamic_states;
    vkGetPhysicalDeviceFeatures2(vk_phy_device_, &phy_features2);

    spdlog::info("dynamic state is enabled: {}",
                 dynamic_states.extendedDynamicState);
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
  compute_queue_index_ = compute_queue_family;
  vk_sample_count_ = get_max_usable_sample_count(phy_props);
}

void VkApp::CreateVkDevice() {
  std::vector<VkDeviceQueueCreateInfo> queue_create_info{};

  std::set<uint32_t> queue_families = {
      graphic_queue_index_,
      present_queue_index_,
      compute_queue_index_,
  };
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

    // for now hard code this extension used in skity vulkan backend
    // required_device_extension.emplace_back("VK_EXT_extended_dynamic_state");
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
  vkGetDeviceQueue(vk_device_, compute_queue_index_, 0, &vk_compute_queue_);
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
  create_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
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

  sampler_image_.resize(image_count);
  for (size_t i = 0; i < sampler_image_.size(); i++) {
    VkImageCreateInfo img_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    img_create_info.imageType = VK_IMAGE_TYPE_2D;
    img_create_info.format = swap_chain_format_;
    img_create_info.extent = {swap_chain_extend_.width,
                              swap_chain_extend_.height, 1};
    img_create_info.mipLevels = 1;
    img_create_info.arrayLayers = 1;
    img_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    img_create_info.samples = vk_sample_count_;
    img_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    img_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(vk_device_, &img_create_info, nullptr,
                      &sampler_image_[i].image) != VK_SUCCESS) {
      spdlog::error("Failed to create VkImage for MSAA");
      exit(-1);
    }

    VkMemoryRequirements mem_reqs{};
    vkGetImageMemoryRequirements(vk_device_, sampler_image_[i].image,
                                 &mem_reqs);
    VkMemoryAllocateInfo mem_alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = GetMemoryType(
        mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(vk_device_, &mem_alloc, nullptr,
                         &sampler_image_[i].memory) != VK_SUCCESS) {
      spdlog::error("Failed to allocate memory for MSAA image");
      exit(-1);
    }

    if (vkBindImageMemory(vk_device_, sampler_image_[i].image,
                          sampler_image_[i].memory, 0) != VK_SUCCESS) {
      spdlog::error("");
      exit(-1);
    }

    // image view for MSAA
    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = sampler_image_[i].image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = swap_chain_format_;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vk_device_, &view_info, nullptr,
                          &sampler_image_[i].image_view) != VK_SUCCESS) {
      spdlog::error("Failed to create vkImageView for MSAA");
      exit(-1);
    }

    sampler_image_[i].format = swap_chain_format_;
  }

  // create image view for color buffer submit to screen
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
  if (!get_support_depth_format(vk_phy_device_, &depth_stencil_format_)) {
    spdlog::error("can not find format support depth and stencil buffer");
    exit(-1);
  }
  // create image and image-view for stencil buffer
  VkImageCreateInfo image_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.format = depth_stencil_format_;
  image_create_info.extent = {swap_chain_extend_.width,
                              swap_chain_extend_.height, 1};
  image_create_info.mipLevels = 1;
  image_create_info.arrayLayers = 1;
  image_create_info.samples = vk_sample_count_;
  image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  stencil_image_.resize(image_count);

  for (size_t i = 0; i < stencil_image_.size(); i++) {
    if (vkCreateImage(vk_device_, &image_create_info, nullptr,
                      &stencil_image_[i].image) != VK_SUCCESS) {
      spdlog::error("Failed to create stencil buffer");
      exit(-1);
    }

    VkMemoryRequirements mem_reqs{};
    vkGetImageMemoryRequirements(vk_device_, stencil_image_[i].image,
                                 &mem_reqs);

    VkMemoryAllocateInfo mem_alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = GetMemoryType(
        mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(vk_device_, &mem_alloc, nullptr,
                         &stencil_image_[i].memory) != VK_SUCCESS) {
      spdlog::error("Failed to allocate stencil buffer memory");
      exit(-1);
    }
    if (vkBindImageMemory(vk_device_, stencil_image_[i].image,
                          stencil_image_[i].memory, 0) != VK_SUCCESS) {
      spdlog::error("Failed to bind stencil buffer memory");
      exit(-1);
    }

    VkImageViewCreateInfo image_view_create_info{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.image = stencil_image_[i].image;
    image_view_create_info.format = depth_stencil_format_;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    if (vkCreateImageView(vk_device_, &image_view_create_info, nullptr,
                          &stencil_image_[i].image_view) != VK_SUCCESS) {
      spdlog::error("Failed to create image view for stencil buffer");
      exit(-1);
    }

    stencil_image_[i].format = depth_stencil_format_;
  }
}

void VkApp::CreateCommandPool() {
  VkCommandPoolCreateInfo create_info{
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  create_info.queueFamilyIndex = graphic_queue_index_;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(vk_device_, &create_info, nullptr, &cmd_pool_) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create Command Pool.");
    exit(-1);
  }
}

void VkApp::CreateCommandBuffers() {
  cmd_buffers_.resize(swap_chain_image_views.size());

  VkCommandBufferAllocateInfo allocate_info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocate_info.commandPool = cmd_pool_;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = static_cast<uint32_t>(cmd_buffers_.size());

  if (vkAllocateCommandBuffers(vk_device_, &allocate_info,
                               cmd_buffers_.data()) != VK_SUCCESS) {
    spdlog::error("Failed to allocate command buffers");
    exit(-1);
  }
}

void VkApp::CreateSyncObjects() {
  VkFenceCreateInfo create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  cmd_fences_.resize(cmd_buffers_.size());

  for (size_t i = 0; i < cmd_fences_.size(); i++) {
    if (vkCreateFence(vk_device_, &create_info, nullptr, &cmd_fences_[i]) !=
        VK_SUCCESS) {
      spdlog::error("Failed to create fence at index : {}", i);
      exit(-1);
    }
  }

  present_semaphore_.resize(cmd_buffers_.size());
  render_semaphore_.resize(cmd_buffers_.size());

  VkSemaphoreCreateInfo semaphore_create_info{
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  for (size_t i = 0; i < present_semaphore_.size(); i++) {
    if (vkCreateSemaphore(vk_device_, &semaphore_create_info, nullptr,
                          &present_semaphore_[i]) != VK_SUCCESS) {
      spdlog::error("Failed to create present semaphore");
      exit(-1);
    }
  }
  for (size_t i = 0; i < render_semaphore_.size(); i++) {
    if (vkCreateSemaphore(vk_device_, &semaphore_create_info, nullptr,
                          &render_semaphore_[i]) != VK_SUCCESS) {
      spdlog::error("Failed to create render semaphore");
      exit(-1);
    }
  }
}

void VkApp::CreateRenderPass() {
  std::array<VkAttachmentDescription, 3> attachments = {};
  // color attachment
  attachments[0].format = swap_chain_format_;
  attachments[0].samples = vk_sample_count_;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // depth stencil attachment
  attachments[1].format = stencil_image_[0].format;
  attachments[1].samples = vk_sample_count_;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // color resolve attachment
  attachments[2].format = swap_chain_format_;
  attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_reference{};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_stencil_reference{};
  depth_stencil_reference.attachment = 1;
  depth_stencil_reference.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference resolve_reference{};
  resolve_reference.attachment = 2;
  resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pDepthStencilAttachment = &depth_stencil_reference;
  subpass.pResolveAttachments = &resolve_reference;

  std::array<VkSubpassDependency, 2> subpass_dependencies{};
  subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependencies[0].dstSubpass = 0;
  subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpass_dependencies[0].dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  subpass_dependencies[1].srcSubpass = 0;
  subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependencies[1].srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo create_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  create_info.attachmentCount = attachments.size();
  create_info.pAttachments = attachments.data();
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = subpass_dependencies.size();
  create_info.pDependencies = subpass_dependencies.data();

  if (vkCreateRenderPass(vk_device_, &create_info, nullptr, &render_pass_) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create render pass!");
    exit(-1);
  }
}

void VkApp::CreateFramebuffers() {
  swap_chain_frame_buffers_.resize(swap_chain_image_views.size());

  std::array<VkImageView, 3> attachments = {};
  for (size_t i = 0; i < swap_chain_frame_buffers_.size(); i++) {
    attachments[0] = sampler_image_[i].image_view;
    attachments[1] = stencil_image_[i].image_view;
    attachments[2] = swap_chain_image_views[i];
    VkFramebufferCreateInfo create_info{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    create_info.renderPass = render_pass_;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.width = swap_chain_extend_.width;
    create_info.height = swap_chain_extend_.height;
    create_info.layers = 1;

    if (vkCreateFramebuffer(vk_device_, &create_info, nullptr,
                            &swap_chain_frame_buffers_[i]) != VK_SUCCESS) {
      spdlog::error("Failed to create frame buffer");
      exit(-1);
    }
  }
}

uint32_t VkApp::GetMemoryType(uint32_t type_bits,
                              VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(vk_phy_device_, &memory_properties);

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
    if ((type_bits & 1) == 1) {
      if ((memory_properties.memoryTypes[i].propertyFlags & properties) ==
          properties) {
        return i;
      }
    }

    type_bits >>= 1;
  }

  return 0;
}

void VkApp::GetCursorPos(double& x, double& y) {
  double mx, my;
  glfwGetCursorPos(window_, &mx, &my);

  x = mx;
  y = my;
}

}  // namespace example
