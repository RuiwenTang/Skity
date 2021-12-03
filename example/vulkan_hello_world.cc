#include <cstring>
#include <example_config.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <skity/codec/data.hpp>
#include <vector>

#include "vk/vk_app.hpp"

class HelloVulkanApp : public example::VkApp {
 public:
  HelloVulkanApp() : VkApp(800, 800, "Hello Vulkan") {}
  ~HelloVulkanApp() override = default;

 protected:
  void OnStart() override {}
  void OnUpdate(float elapsed_time) override {}
  void OnDestroy() override {}

 private:
};

int main(int argc, const char** argv) {
  HelloVulkanApp app;
  app.Run();
  return 0;
}
