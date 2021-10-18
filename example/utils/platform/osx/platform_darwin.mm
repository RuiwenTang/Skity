#include "utils/platform/platform_glfw.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#import <Cocoa/Cocoa.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#import <QuartzCore/CAMetalLayer.h>
#include <vulkan/vulkan.hpp>

namespace example {

static void* getCAMetaLayer(void* window) {
  NSWindow* ns_window = (__bridge NSWindow*)window;
  NSView* view = [ns_window contentView];

  assert([view isKindOfClass:[NSView class]]);
  if (![view.layer isKindOfClass:[CAMetalLayer class]]) {
    [view setLayer:[CAMetalLayer layer]];
    [view setWantsLayer:YES];
  }

  return (__bridge void*)view.layer;
}

class PlatformDarwin : public PlatformGLFW {
 public:
  PlatformDarwin() = default;
  ~PlatformDarwin() override = default;

  void* GetWindowSurface(void* window) override {
    return getCAMetaLayer(glfwGetCocoaWindow(reinterpret_cast<GLFWwindow*>(window)));
  }

  std::vector<const char*> GetRequiredInstanceExtensions() override {
    auto extensions = PlatformGLFW::GetRequiredInstanceExtensions();
    extensions.emplace_back("VK_EXT_metal_surface");
    return extensions;
  }

  void* CreateSurface(void* instance, void* window) override {
    VkMetalSurfaceCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    create_info.pLayer = static_cast<const CAMetalLayer*>(GetWindowSurface(window));

    VkSurfaceKHR _surface = nullptr;
    if (vkCreateMetalSurfaceEXT(static_cast<VkInstance>(instance), &create_info, nullptr,
                                &_surface) != VK_SUCCESS) {
      return nullptr;
    }

    return _surface;
  }
};

std::unique_ptr<Platform> Platform::CreatePlatform() {
  return std::unique_ptr<Platform>(new PlatformDarwin);
}

}