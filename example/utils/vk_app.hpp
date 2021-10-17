#ifndef EXAMPLE_UTILS_VK_APP_HPP
#define EXAMPLE_UTILS_VK_APP_HPP

#include <memory>
#include <string>

namespace example {

class Platform;
class VkApp {
 public:
  VkApp(int32_t width, int32_t height, std::string name)
      : width_(width), height_(height), window_name_(name) {}
  virtual ~VkApp();

 private:
  int32_t width_ = 0;
  int32_t height_ = 0;
  std::string window_name_;
  std::unique_ptr<Platform> platform_;
};
}  // namespace example

#endif  // EXAMPLE_UTILS_VK_APP_HPP
