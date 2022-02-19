#include <skity/skity.hpp>

#include "example_config.hpp"
#include "gl/gl_app.hpp"
#include "perf.hpp"

void load_images(std::vector<std::shared_ptr<skity::Pixmap>>& images);

void render_frame_demo(
    skity::Canvas* canvas,
    std::vector<std::shared_ptr<skity::Pixmap>> const& images,
    std::shared_ptr<skity::Typeface> const& typeface,
    std::shared_ptr<skity::Typeface> const& emoji, float mx, float my,
    float width, float height, float t);

class GLFrameApp : public example::GLApp {
 public:
  GLFrameApp()
      : example::GLApp(1000, 600, "GL Frame example", {0.3f, 0.3f, 0.32f, 1.f}),
        fpsGraph(Perf::GRAPH_RENDER_FPS, "Frame Time"),
        cpuGraph(Perf::GRAPH_RENDER_MS, "CPU Time") {}
  ~GLFrameApp() override = default;

 protected:
  void OnStart() override {
    load_images(images_);
    typeface_ =
        skity::Typeface::MakeFromFile(EXAMPLE_IMAGE_ROOT "/Roboto-Regular.ttf");

    emoji_typeface_ = skity::Typeface::MakeFromFile(EXAMPLE_IMAGE_ROOT
                                                    "/NotoEmoji-Regular.ttf");

    time_ = prev_time_ = glfwGetTime();
  }

  void OnUpdate(float elapsed_time) override {
    double mx, my;
    GetCursorPos(mx, my);

    time_ = glfwGetTime();

    double dt = time_ - prev_time_;
    prev_time_ = time_;

    render_frame_demo(GetCanvas(), images_, typeface_, emoji_typeface_, mx, my,
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
  std::shared_ptr<skity::Typeface> emoji_typeface_ = {};
  double time_ = 0;
  double prev_time_ = 0;
  double cpu_time_ = 0;
  double dt_ = 0;
  Perf fpsGraph;
  Perf cpuGraph;
};

int main(int argc, const char** argv) {
  GLFrameApp app;
  app.Run();

  return 0;
}