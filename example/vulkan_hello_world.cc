#include <iostream>
#include <vector>

#include "utils/vk_app.hpp"

class HelloVulkanApp : public example::VkApp {
 public:
  HelloVulkanApp() : VkApp(800, 800, "Hello Vulkan") {}
  ~HelloVulkanApp() override = default;

 protected:
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
