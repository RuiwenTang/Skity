#include "utils/vk_app.hpp"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <iostream>
#include <limits>
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
  // create vulkan surface
  CreateSurface();
  // pick vulkan device
  PickPhysicalDevice();
  // create logical vulkan device
  CreateLogicalDevice();
  // create swap chain
  CreateSwapChain();
  CreateSwapChainImageView();
  // command buffer
  CreateCommandPoolAndBuffer();
  CreateRenderPass();
  CreateFramebuffer();
  CreateSyncObject();

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

  std::vector<const char*> instance_extension_names =
      platform_->GetRequiredInstanceExtensions();

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

void VkApp::CreateSurface() {
  auto raw_surface = platform_->CreateSurface(vk_instance_.get(), window_);

  if (raw_surface == nullptr) {
    std::cerr << "Failed to creat Vulkan surface" << std::endl;
    exit(-1);
  }

  vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderStatic> _deleter(
      vk_instance_.get(), nullptr, vk_dispatch_);

  vk_surface_ = vk::UniqueSurfaceKHR(
      vk::SurfaceKHR(static_cast<VkSurfaceKHR>(raw_surface)), _deleter);
}

void VkApp::PickPhysicalDevice() {
  auto physical_devices = vk_instance_->enumeratePhysicalDevices(vk_dispatch_);

  int32_t graphic_queue_family_index = -1;
  int32_t present_queue_family_index = -1;

  for (const auto& vk_pd : physical_devices.value) {
    auto property = vk_pd.getProperties(vk_dispatch_);
    std::cout << "GPU : " << property.deviceName << std::endl;

    const auto& queue_family_properties =
        vk_pd.getQueueFamilyProperties(vk_dispatch_);

    graphic_queue_family_index = std::distance(
        queue_family_properties.begin(),
        std::find_if(queue_family_properties.begin(),
                     queue_family_properties.end(),
                     [](vk::QueueFamilyProperties props) {
                       return props.queueFlags & vk::QueueFlagBits::eGraphics;
                     }));

    if (graphic_queue_family_index < queue_family_properties.size()) {
      vk_physical_device_ = vk_pd;
      break;
    }
  }

  if (!vk_physical_device_) {
    std::cerr << "Can not find GPU support graphics" << std::endl;
    exit(-1);
  }

  auto graphic_queue_support_present = vk_physical_device_.getSurfaceSupportKHR(
      graphic_queue_family_index, vk_surface_.get(), vk_dispatch_);

  if (graphic_queue_support_present.result == vk::Result::eSuccess) {
    present_queue_family_index = graphic_queue_family_index;
  } else {
    size_t prop_count =
        vk_physical_device_.getQueueFamilyProperties(vk_dispatch_).size();
    for (size_t i = 0; i < prop_count; i++) {
      if (vk_physical_device_
              .getSurfaceSupportKHR(i, vk_surface_.get(), vk_dispatch_)
              .result == vk::Result::eSuccess) {
        present_queue_family_index = i;
        break;
      }
    }
  }

  if (present_queue_family_index == -1) {
    std::cerr << "Failed to find a present queue on GPU" << std::endl;
    exit(-1);
  }

  vk_graphic_queue_index_ = graphic_queue_family_index;
  vk_present_queue_index_ = present_queue_family_index;
}

void VkApp::CreateLogicalDevice() {
  std::vector<const char*> required_device_layer{};
  std::vector<const char*> required_device_extension{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  // Fix VUID-VkDeviceCreateInfo-pProperties-04451
  {
    auto properties = vk_physical_device_.enumerateDeviceExtensionProperties(
        std::string(""), vk_dispatch_);

    if (properties.result == vk::Result::eSuccess) {
      auto it =
          std::find_if(properties.value.begin(), properties.value.end(),
                       [](vk::ExtensionProperties const& prop) {
                         return std::strcmp(prop.extensionName,
                                            "VK_KHR_portability_subset") == 0;
                       });
      if (it != properties.value.end()) {
        required_device_extension.emplace_back("VK_KHR_portability_subset");
      }
    }
  }

  float queue_priority = 0.f;
  vk::DeviceQueueCreateInfo device_queue_create_info{
      vk::DeviceQueueCreateFlags{},
      static_cast<uint32_t>(vk_graphic_queue_index_), 1, &queue_priority};

  vk::DeviceCreateInfo create_info{
      vk::DeviceCreateFlags{}, device_queue_create_info, required_device_layer,
      required_device_extension};

  auto create_ret = vk_physical_device_.createDeviceUnique(create_info, nullptr,
                                                           vk_dispatch_);

  if (create_ret.result != vk::Result::eSuccess) {
    std::cerr << "Failed to create logical device" << std::endl;
    exit(-1);
  }

  vk_device_ = std::move(create_ret.value);
}

void VkApp::CreateSwapChain() {
  auto formats =
      vk_physical_device_.getSurfaceFormatsKHR(vk_surface_.get(), vk_dispatch_);

  if (formats.result != vk::Result::eSuccess) {
    std::cerr << "Can not get surface formats" << std::endl;
    exit(-1);
  }

  auto window_size = platform_->GetWindowSize(window_);
  vk::Format format = (formats.value.front().format == vk::Format::eUndefined)
                          ? vk::Format::eB8G8R8A8Unorm
                          : formats.value.front().format;

  auto surface_capabilities = vk_physical_device_.getSurfaceCapabilitiesKHR(
      vk_surface_.get(), vk_dispatch_);

  // surface size
  VkExtent2D swap_chain_extent;
  if (surface_capabilities.value.currentExtent.width ==
      std::numeric_limits<uint32_t>::max()) {
    swap_chain_extent.width =
        glm::clamp(static_cast<uint32_t>(window_size[0]),
                   surface_capabilities.value.minImageExtent.width,
                   surface_capabilities.value.maxImageExtent.width);

    swap_chain_extent.height =
        glm::clamp(static_cast<uint32_t>(window_size[1]),
                   surface_capabilities.value.minImageExtent.height,
                   surface_capabilities.value.maxImageExtent.height);
  } else {
    swap_chain_extent = surface_capabilities.value.currentExtent;
  }

  // present mode
  vk::PresentModeKHR swap_chain_present_mode = vk::PresentModeKHR::eFifo;

  vk::SurfaceTransformFlagBitsKHR present_transform =
      (surface_capabilities.value.supportedTransforms &
       vk::SurfaceTransformFlagBitsKHR::eIdentity)
          ? vk::SurfaceTransformFlagBitsKHR::eIdentity
          : surface_capabilities.value.currentTransform;

  vk::CompositeAlphaFlagBitsKHR composite_alpha;
  if (surface_capabilities.value.supportedCompositeAlpha &
      vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) {
    composite_alpha = vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
  } else if (surface_capabilities.value.supportedCompositeAlpha &
             vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) {
    composite_alpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
  } else if (surface_capabilities.value.supportedCompositeAlpha &
             vk::CompositeAlphaFlagBitsKHR::eInherit) {
    composite_alpha = vk::CompositeAlphaFlagBitsKHR::eInherit;
  } else {
    composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  }

  vk::SwapchainCreateInfoKHR swap_chain_create_info{
      vk::SwapchainCreateFlagsKHR{},
      vk_surface_.get(),
      surface_capabilities.value.minImageCount,
      format,
      vk::ColorSpaceKHR::eSrgbNonlinear,
      swap_chain_extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc,
      vk::SharingMode::eExclusive,
      {},
      present_transform,
      composite_alpha,
      swap_chain_present_mode,
      true,
      nullptr};

  std::array<uint32_t, 2> queue_family_indices = {
      static_cast<uint32_t>(vk_graphic_queue_index_),
      static_cast<uint32_t>(vk_present_queue_index_)};

  if (queue_family_indices[0] != queue_family_indices[1]) {
    // if the graphic queue and present queue is different, need to make swap
    // chain can transfer images or sharing images
    swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
    swap_chain_create_info.queueFamilyIndexCount = 2;
    swap_chain_create_info.pQueueFamilyIndices = queue_family_indices.data();
  } else {
    swap_chain_create_info.queueFamilyIndexCount = 1;
    swap_chain_create_info.pQueueFamilyIndices = queue_family_indices.data();
  }

  auto swap_chain_ret = vk_device_->createSwapchainKHRUnique(
      swap_chain_create_info, nullptr, vk_dispatch_);

  if (swap_chain_ret.result != vk::Result::eSuccess) {
    exit(-1);
  }

  vk_swap_chain_ = std::move(swap_chain_ret.value);

  vk_graphic_queue_ =
      vk_device_->getQueue(vk_graphic_queue_index_, 0, vk_dispatch_);

  vk_present_queue_ =
      vk_device_->getQueue(vk_present_queue_index_, 0, vk_dispatch_);

  vk_color_attachment_format_ = format;
}

void VkApp::CreateSwapChainImageView() {
  auto images =
      vk_device_->getSwapchainImagesKHR(vk_swap_chain_.get(), vk_dispatch_);
  vk_swap_chain_image_view_.reserve(images.value.size());

  vk::ComponentMapping component_mapping{
      vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};

  vk::ImageSubresourceRange sub_resource_range{vk::ImageAspectFlagBits::eColor,
                                               0, 1, 0, 1};

  for (auto image : images.value) {
    vk::ImageViewCreateInfo image_view_create_info{{},
                                                   image,
                                                   vk::ImageViewType::e2D,
                                                   vk_color_attachment_format_,
                                                   component_mapping,
                                                   sub_resource_range};

    vk_swap_chain_image_view_.emplace_back(vk_device_->createImageViewUnique(
        image_view_create_info, nullptr, vk_dispatch_));
  }
}

void VkApp::CreateCommandPoolAndBuffer() {
  vk::CommandPoolCreateInfo pool_create_info{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      static_cast<uint32_t>(vk_graphic_queue_index_)};

  auto pool_create_ret = vk_device_->createCommandPoolUnique(
      pool_create_info, nullptr, vk_dispatch_);

  if (pool_create_ret.result != vk::Result::eSuccess) {
    std::cerr << "Failed to create Vulkan Command Pool" << std::endl;
    exit(-1);
  }

  vk_command_pool_ = std::move(pool_create_ret.value);

  vk::CommandBufferAllocateInfo buffer_allocate_info{
      vk_command_pool_.get(), vk::CommandBufferLevel::ePrimary, 1};

  auto buffer_allocate_ret = vk_device_->allocateCommandBuffersUnique(
      buffer_allocate_info, vk_dispatch_);

  if (buffer_allocate_ret.result != vk::Result::eSuccess) {
    exit(-1);
  }

  vk_command_buffer_ = std::move(buffer_allocate_ret.value.front());
}

void VkApp::CreateRenderPass() {
  std::vector<vk::AttachmentDescription> color_attachment_descriptions;
  color_attachment_descriptions.emplace_back(
      vk::AttachmentDescription{{},
                                vk_color_attachment_format_,
                                vk::SampleCountFlagBits::e1,
                                vk::AttachmentLoadOp::eClear,
                                vk::AttachmentStoreOp::eStore,
                                vk::AttachmentLoadOp::eDontCare,
                                vk::AttachmentStoreOp::eDontCare,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::ePresentSrcKHR});

  vk::AttachmentReference color_attachment_ref{
      0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription sub_pass_desc{
      {},
      vk::PipelineBindPoint::eGraphics,
      {} /* input attachment */,
      color_attachment_ref,
  };

  std::vector<vk::SubpassDependency> sub_pass_dependencies{};

  vk::RenderPassCreateInfo render_pass_create_info{
      {}, color_attachment_descriptions, sub_pass_desc, sub_pass_dependencies};

  auto create_ret = vk_device_->createRenderPass(render_pass_create_info,
                                                 nullptr, vk_dispatch_);

  if (create_ret.result != vk::Result::eSuccess) {
    exit(-1);
  }
}

void VkApp::CreateFramebuffer() {
  vk_swap_chain_frame_buffer_.reserve(vk_swap_chain_image_view_.size());
  std::array<vk::ImageView, 1> attachments;
  for (const auto& image_view : vk_swap_chain_image_view_) {
    attachments[0] = image_view.get();

    auto create_ret = vk_device_->createFramebufferUnique(
        vk::FramebufferCreateInfo{}, nullptr, vk_dispatch_);
    if (create_ret.result != vk::Result::eSuccess) {
      exit(-1);
    }

    vk_swap_chain_frame_buffer_.emplace_back(std::move(create_ret.value));
  }
}

void VkApp::CreateSyncObject() {
  auto semaphore_ret = vk_device_->createSemaphoreUnique(
      vk::SemaphoreCreateInfo{}, nullptr, vk_dispatch_);

  assert(semaphore_ret.result == vk::Result::eSuccess);
  vk_image_acquired_semaphore_ = std::move(semaphore_ret.value);

  auto fence_ret = vk_device_->createFenceUnique(vk::FenceCreateInfo{}, nullptr,
                                                 vk_dispatch_);

  assert(fence_ret.result == vk::Result::eSuccess);
  vk_draw_fence_ = std::move(fence_ret.value);
}

}  // namespace example

// ----------------------------------------------------------------------------

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