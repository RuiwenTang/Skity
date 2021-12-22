#include <skity/skity.hpp>

#include "example_config.hpp"
#include "perf.hpp"
#include "vk/vk_app.hpp"

void load_images(std::vector<std::shared_ptr<skity::Pixmap>>& images);

void render_frame_demo(
    skity::Canvas* canvas,
    std::vector<std::shared_ptr<skity::Pixmap>> const& images,
    skity::SVGDom* svg, std::shared_ptr<skity::Typeface> const& typeface,
    float mx, float my, float width, float height, float t);

class VkFrameApp : public example::VkApp {
 public:
  VkFrameApp()
      : example::VkApp(1000, 600, "Vulkan Frame example",
                       {0.3f, 0.3f, 0.32f, 1.f}),
        fpsGraph(Perf::GRAPH_RENDER_FPS, "Frame Time"),
        cpuGraph(Perf::GRAPH_RENDER_MS, "CPU Time") {}
  ~VkFrameApp() override = default;

 protected:
  void OnStart() override {
    VkApp::OnStart();
    // glfwSwapInterval(0);
    load_images(images_);
    typeface_ =
        skity::Typeface::MakeFromFile(EXAMPLE_IMAGE_ROOT "/Roboto-Regular.ttf");

    time_ = prev_time_ = glfwGetTime();
  }

  void OnUpdate(float elapsed_time) override {
    double mx, my;
    GetCursorPos(mx, my);

    time_ = glfwGetTime();

    double dt = time_ - prev_time_;
    prev_time_ = time_;

    render_frame_demo(GetCanvas(), images_, nullptr, typeface_, mx, my,
                      ScreenWidth(), ScreenHeight(), time_);

    cpu_time_ = glfwGetTime() - time_;
    fpsGraph.RenderGraph(GetCanvas(), 5, 5);
    cpuGraph.RenderGraph(GetCanvas(), 5 + 200 + 5, 5);

    GetCanvas()->flush();
    fpsGraph.UpdateGraph(dt);
    cpuGraph.UpdateGraph(cpu_time_);
  }

 private:
  std::vector<std::shared_ptr<skity::Pixmap>> images_ = {};
  std::shared_ptr<skity::Typeface> typeface_ = {};
  double time_ = 0;
  double prev_time_ = 0;
  double cpu_time_ = 0;
  double dt_ = 0;
  Perf fpsGraph;
  Perf cpuGraph;
};

int main(int argc, const char** argv) {
  VkFrameApp app;
  app.Run();

  return 0;
}