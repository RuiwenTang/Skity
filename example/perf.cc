#include <glad/glad.h>
// should after glad.h
#include <GLFW/glfw3.h>

#include <cstdio>
#include <skity/graphic/paint.hpp>

#include "perf.hpp"

#ifndef GL_ARB_timer_query
#define GL_TIME_ELAPSED 0x88BF
#endif

Perf::Perf(GraphRenderStyle style, std::string name)
    : name_(std::move(name)),
      style_(style),
      head_(0),
      supported_(false),
      cur_(0),
      ret_(0) {}

Perf::~Perf() {}

void Perf::StartGPUTimer() {}

void Perf::StopGPUTimer(float *times, int maxTimes) {}

void Perf::UpdateGraph(float frameTime) {
  head_ = (head_ + 1) % values_.size();
  values_[head_] = frameTime;
}

float Perf::GetGraphAverage() {
  float avg = 0.f;
  for (auto v : values_) {
    avg += v;
  }
  return avg / (float)values_.size();
}

void Perf::RenderGraph(skity::Canvas *canvas, float x, float y) {
  float avg, w, h;
  avg = GetGraphAverage();

  w = 200;
  h = 35;
  skity::Paint paint;
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(128, 0, 0, 0));
  skity::Rect rect = skity::Rect::MakeXYWH(x, y, w, h);
  canvas->drawRect(rect, paint);

  skity::Path path;
  path.moveTo(x, y + h);
  if (style_ == GRAPH_RENDER_FPS) {
    for (int32_t i = 0; i < values_.size(); i++) {
      float v = 1.f / (0.00001f + values_[(head_ + i) % GRAPH_HISTORY_COUNT]);
      float vx, vy;
      if (v > 80.f) {
        v = 80.f;
      }
      vx = x + ((float)i / (GRAPH_HISTORY_COUNT - 1)) * w;
      vy = y + h - ((v / 80.f) * h);
      path.lineTo(vx, vy);
    }
  } else if (style_ == GRAPH_RENDER_PERCENT) {
    for (int32_t i = 0; i < values_.size(); i++) {
      float v = values_[(head_ + i) % GRAPH_HISTORY_COUNT] * 1.f;
      float vx, vy;
      if (v > 100.f) {
        v = 100.f;
      }
      vx = x + ((float)i / (GRAPH_HISTORY_COUNT - 1)) * w;
      vy = y + h - ((v / 100.0f) * h);
      path.lineTo(vx, vy);
    }
  } else {
    for (int32_t i = 0; i < values_.size(); i++) {
      float v = values_[(head_ + i) % GRAPH_HISTORY_COUNT] * 1000.f;
      float vx, vy;
      if (v > 20.f) {
        v = 20.f;
      }
      vx = x + ((float)i / (GRAPH_HISTORY_COUNT - 1)) * w;
      vy = y + h - ((v / 20.0f) * h);
      path.lineTo(vx, vy);
    }
  }

  path.lineTo(x + w, y + h);
  path.close();
  paint.setColor(skity::ColorSetARGB(128, 255, 192, 0));
  canvas->drawPath(path, paint);

  if (!name_.empty()) {
    paint.setTextSize(12.f);
    paint.setColor(skity::ColorSetARGB(192, 240, 240, 240));
    canvas->drawSimpleText(name_.c_str(), x + 3, y + 3 + 14.f, paint);
  }

  char str[64];

  if (style_ == GRAPH_RENDER_FPS) {
    paint.setTextSize(15.f);
    paint.setColor(skity::ColorSetARGB(255, 240, 240, 240));
    std::sprintf(str, "%.2f FPS", 1.0f / avg);
    canvas->drawSimpleText(str, x + 100.f, y + 3.f + 15.f, paint);

    paint.setTextSize(13.f);
    paint.setColor(skity::ColorSetARGB(160, 240, 240, 240));
    sprintf(str, "%.2f ms", avg * 1000.f);
    canvas->drawSimpleText(str, x + 100.f, y + 3.f + 15.f + 14.f,
                           paint);
  } else if (style_ == GRAPH_RENDER_PERCENT) {
    paint.setTextSize(15.f);
    paint.setColor(skity::ColorSetARGB(255, 240, 240, 240));
    sprintf(str, "%.1f %%", avg * 1.f);
    canvas->drawSimpleText(str, x + 60.f, y + 3.f + 15.f, paint);
  } else {
    paint.setTextSize(15.f);
    paint.setColor(skity::ColorSetARGB(255, 240, 240, 240));
    sprintf(str, "%.2f ms", avg * 1000.f);
    canvas->drawSimpleText(str, x + 100.f, y + 3.f + 15.f, paint);
  }
}