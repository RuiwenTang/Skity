#ifndef SKITY_PERF_HPP
#define SKITY_PERF_HPP

#include <array>
#include <skity/render/canvas.hpp>
#include <string>

class Perf final {
 public:
  enum GraphRenderStyle {
    GRAPH_RENDER_FPS,
    GRAPH_RENDER_MS,
    GRAPH_RENDER_PERCENT,
  };

  enum {
    GRAPH_HISTORY_COUNT = 100,
    GPU_QUERY_COUNT = 5,
  };

  Perf(GraphRenderStyle style, std::string name);
  ~Perf();

  void UpdateGraph(float frameTime);
  float GetGraphAverage();
  void RenderGraph(skity::Canvas* canvas, float x, float y);

  void StartGPUTimer();
  void StopGPUTimer(float* times, int maxTimes);

 private:
  std::string name_;
  GraphRenderStyle style_;
  std::array<float, GRAPH_HISTORY_COUNT> values_ {};
  int32_t head_;
  bool supported_;
  int32_t cur_;
  int32_t ret_;
  std::array<uint32_t, GPU_QUERY_COUNT> queries_ {};
};

#endif  // SKITY_PERF_HPP
