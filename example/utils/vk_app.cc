#include "utils/vk_app.hpp"

#include <iostream>
#include <vector>

#include "utils/vk_platform.hpp"

namespace example {

static PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT =
    nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT =
    nullptr;

static void load_debug_pfn_if_need(vk::Instance const& instance) {
  if (pfnVkCreateDebugUtilsMessengerEXT == nullptr) {
    pfnVkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
  }

  if (pfnVkCreateDebugUtilsMessengerEXT == nullptr) {
    std::cerr << "Unable to load pfnVkCreateDebugUtilsMessengerEXT function"
              << std::endl;
    exit(-1);
  }

  if (pfnVkDestroyDebugUtilsMessengerEXT == nullptr) {
    pfnVkDestroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
  }

  if (pfnVkDestroyDebugUtilsMessengerEXT == nullptr) {
    std::cerr << "Unable to load pfnVkDestroyDebugUtilsMessengerEXT function"
              << std::endl;
    exit(-1);
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_message_func(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                   VkDebugUtilsMessageTypeFlagsEXT message_type,
                   VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
                   void* /*pUserData*/) {
  std::string message;
  message +=
      vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(
          message_severity)) +
      ": " +
      vk::to_string(
          static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_type)) +
      ":\n";

  message += std::string("\t") + "messageIDName =<" +
             (p_callback_data->pMessageIdName ? p_callback_data->pMessageIdName
                                              : "null") +
             ">\n";

  message += std::string("\t") + "messageIDNumber =<" +
             std::to_string(p_callback_data->messageIdNumber) + ">\n";

  message +=
      std::string("\t") + "message =<" + p_callback_data->pMessage + ">\n";

  std::cout << message << std::endl;
  return false;
}

static bool check_layers(std::vector<const char*> const& layers,
                         std::vector<vk::LayerProperties> const& properties) {
  // return true if all layers are listed in the properties
  return std::all_of(
      layers.begin(), layers.end(), [properties](const char* name) {
        return std::find_if(properties.begin(), properties.end(),
                            [name](vk::LayerProperties const& property) {
                              return std::strcmp(property.layerName, name) == 0;
                            }) != properties.end();
      });
}

VkApp::VkApp(int32_t width, int32_t height, std::string name)

    : width_(width),
      height_(height),
      window_name_(std::move(name)),
      window_(nullptr),
      platform_(Platform::CreatePlatform()),
      vk_dispatch_(vk::DispatchLoaderStatic()) {}

VkApp::~VkApp() = default;

void VkApp::Run() {
  platform_->StartUp();

  window_ = platform_->CreateWindow(width_, height_, window_name_);
  if (!window_) {
    exit(-1);
  }
  // create vulkan instance
  CreateVkInstance();
  // pick vulkan device
  PickPhysicalDevice();

  this->OnCreate();

  platform_->StartEventLoop(this);

  this->OnDestroy();

  platform_->CleanUp();
  platform_->DestroyWindow(window_);
  platform_->CleanUp();
}

void VkApp::Update(float elapsed_time) {
  this->OnUpdate(elapsed_time);
  platform_->SwapBuffers(window_);
}

void VkApp::CreateVkInstance() {
  bool find_validation_layer = false;
  auto instance_layer_properties = vk::enumerateInstanceLayerProperties();
  if (instance_layer_properties.result != vk::Result::eSuccess) {
    exit(-1);
  }

  std::vector<const char*> instance_layer_names{};
  instance_layer_names.emplace_back("VK_LAYER_KHRONOS_validation");
  if (!check_layers(instance_layer_names, instance_layer_properties.value)) {
    std::cerr << "Failed to load validation layer" << std::endl;
    instance_layer_names.clear();
  } else {
    find_validation_layer = true;
  }

  std::vector<const char*> instance_extension_names{
      VK_KHR_SURFACE_EXTENSION_NAME};

  if (find_validation_layer) {
    instance_extension_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  vk::ApplicationInfo app_info{window_name_.c_str(), VK_MAKE_VERSION(0, 0, 1),
                               "Skity", VK_MAKE_VERSION(0, 0, 1),
                               VK_API_VERSION_1_2};

  vk::InstanceCreateInfo instance_create_info{
      {}, &app_info, instance_layer_names, instance_extension_names};

  auto vk_result =
      vk::createInstanceUnique(instance_create_info, nullptr, vk_dispatch_);
  if (vk_result.result != vk::Result::eSuccess) {
    std::cerr << "Failed to create VkInstance" << std::endl;
    exit(-1);
  }

  vk_instance_ = std::move(vk_result.value);

  if (find_validation_layer) {
    load_debug_pfn_if_need(vk_instance_.get());

    vk::DebugUtilsMessageSeverityFlagsEXT severity_flags =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    vk::DebugUtilsMessageTypeFlagsEXT message_type_flags =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    auto debug_messenger_result =
        vk_instance_->createDebugUtilsMessengerEXTUnique(
            vk::DebugUtilsMessengerCreateInfoEXT{
                {},
                severity_flags,
                message_type_flags,
                reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(
                    &debug_message_func)},
            nullptr, vk_dispatch_);

    if (debug_messenger_result.result != vk::Result::eSuccess) {
      exit(-1);
    }

    vk_debug_messenger_ = std::move(debug_messenger_result.value);
  }
}

void VkApp::PickPhysicalDevice() {}

}  // namespace example

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger) {
  return example::pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo,
                                                    pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* pAllocator) {
  return example::pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger,
                                                     pAllocator);
}