#include <glad/glad.h>
// should after glad.h
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <example_config.hpp>
#include <glm/glm.hpp>
#include <skity/codec/codec.hpp>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/effect/shader.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>
#include <string>

#include "perf.hpp"

void render_frame_demo(
    skity::Canvas* canvas,
    std::vector<std::shared_ptr<skity::Pixmap>> const& images, float mx,
    float my, float width, float height, float t);

void draw_eyes(skity::Canvas* canvas, float x, float y, float w, float h,
               float mx, float my, float t);

void draw_graph(skity::Canvas* canvas, float x, float y, float w, float h,
                float t);

void draw_color_wheel(skity::Canvas* canvas, float x, float y, float w, float h,
                      float t);

void draw_lines(skity::Canvas* canvas, float x, float y, float w, float h,
                float t);

void draw_widths(skity::Canvas* canvas, float x, float y, float width);

void draw_caps(skity::Canvas* canvas, float x, float y, float width);

void draw_scissor(skity::Canvas* canvas, float x, float y, float t);

void draw_window(skity::Canvas* canvas, const char* title, float x, float y,
                 float w, float h);

void draw_search_box(skity::Canvas* canvas, const char* title, float x, float y,
                     float w, float h);

void draw_drop_down(skity::Canvas* canvas, const char* text, float x, float y,
                    float w, float h);

void draw_label(skity::Canvas* canvas, const char* text, float x, float y,
                float w, float h);

void draw_edit_box_base(skity::Canvas* canvas, float x, float y, float w,
                        float h);

void draw_edit_box(skity::Canvas* canvas, const char* text, float x, float y,
                   float w, float h);

void draw_check_box(skity::Canvas* canvas, const char* text, float x, float y,
                    float w, float h);

void draw_button(skity::Canvas* canvas, const char* pre_icon, const char* text,
                 float x, float y, float w, float h, skity::Color col);

void draw_edit_box_num(skity::Canvas* canvas, const char* text,
                       const char* units, float x, float y, float w, float h);

void draw_slider(skity::Canvas* canvas, float pos, float x, float y, float w,
                 float h);

void load_images(std::vector<std::shared_ptr<skity::Pixmap>>& images);

void draw_thumbnails(skity::Canvas* canvas,
                     std::vector<std::shared_ptr<skity::Pixmap>> const& images,
                     float x, float y, float w, float h, float t);

void draw_spinner(skity::Canvas* canvas, float cx, float cy, float r, float t);

int main(int argc, const char** argv) {
  GLFWwindow* window = nullptr;

  int32_t width = 1000;
  int32_t height = 600;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // multisample
  glfwWindowHint(GLFW_SAMPLES, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  window =
      glfwCreateWindow(width, height, "Skity frame demo", nullptr, nullptr);

  if (window == nullptr) {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    exit(-3);
  }

  glClearColor(0.3f, 0.3f, 0.32f, 1.f);
  glClearStencil(0x0);
  glStencilMask(0xFF);
  // blend is need for anti-alias
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  auto canvas = skity::Canvas::MakeGLCanvas(0, 0, width, height,
                                            (void*)glfwGetProcAddress);

  std::vector<std::shared_ptr<skity::Pixmap>> images{};
  load_images(images);

  glfwSwapInterval(0);

  Perf fpsGraph(Perf::GRAPH_RENDER_FPS, "Frame Time");
  Perf cpuGraph(Perf::GRAPH_RENDER_MS, "CPU Time");
  Perf gpuGraph(Perf::GRAPH_RENDER_MS, "GPU Time");

  double mx, my, t, dt, prevt = 0, cpuTime = 0;
  glfwSetTime(0);
  prevt = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glfwGetCursorPos(window, &mx, &my);

    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    t = glfwGetTime();
    dt = t - prevt;
    prevt = t;

    render_frame_demo(canvas.get(), images, mx, my, width, height, t);

    fpsGraph.RenderGraph(canvas.get(), 5, 5);
    cpuGraph.RenderGraph(canvas.get(), 5 + 200 + 5, 5);
    canvas->flush();

    cpuTime = glfwGetTime() - t;
    fpsGraph.UpdateGraph(dt);
    cpuGraph.UpdateGraph(cpuTime);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void render_frame_demo(
    skity::Canvas* canvas,
    std::vector<std::shared_ptr<skity::Pixmap>> const& images, float mx,
    float my, float width, float height, float t) {
  float x, y, popy;

  draw_eyes(canvas, width - 250, 50, 150, 100, mx, my, t);
  // draw_paragraph
  draw_graph(canvas, 0, height / 2.f, width, height / 2.f, t);
  draw_color_wheel(canvas, width - 300, height - 300, 250.f, 250.f, t);

  // Line joints
  draw_lines(canvas, 120, height - 50, 600, 50, t);
  draw_widths(canvas, 10, 50, 30);
  // Line caps
  draw_caps(canvas, 10, 300, 30);

  draw_scissor(canvas, 50, height - 80, t);

  // widgets
  draw_window(canvas, "Widgets 'n Stuff", 50, 50, 300, 400);
  x = 60;
  y = 95;
  draw_search_box(canvas, "Search", x, y, 280, 25);
  y += 40;
  draw_drop_down(canvas, "Effects", x, y, 280, 28);
  popy = y + 14.f;
  y += 45.f;

  // Form
  draw_label(canvas, "Login", x, y, 280, 20);
  y += 25.f;
  draw_edit_box(canvas, "Email", x, y, 280, 28);
  y += 35.f;
  draw_edit_box(canvas, "Password", x, y, 280, 28);
  y += 38.f;
  draw_check_box(canvas, "Remember me", x, y, 140, 28);
  draw_button(canvas, "\ufafb" /* login */, "Sign in", x + 138.f, y, 140, 28,
              skity::ColorSetARGB(255, 0, 96, 128));

  y += 45.f;
  // Slider
  draw_label(canvas, "Diameter", x, y, 280, 20);
  y += 25.f;
  draw_edit_box_num(canvas, "123.00", "px", x + 180, y, 100, 28);
  draw_slider(canvas, 0.4f, x, y, 170, 28);

  y += 55;
  draw_button(canvas, "\uf1f8" /* trash */, "Delete", x, y, 160, 28,
              skity::ColorSetARGB(255, 128, 16, 8));
  draw_button(canvas, nullptr, "Cancel", x + 170, y, 110, 28,
              skity::Color_TRANSPARENT);

  // Thumbnails box
  if (!images.empty()) {
    draw_thumbnails(canvas, images, 365, popy - 30, 160, 300, t);
  }
}

void draw_eyes(skity::Canvas* canvas, float x, float y, float w, float h,
               float mx, float my, float t) {
  skity::Paint gloss, bg;
  float ex = w * 0.23f;
  float ey = h * 0.5f;
  float lx = x + ex;
  float ly = y + ey;
  float rx = x + w - ex;
  float ry = y + ey;
  float dx, dy, d;
  float br = (ex < ey ? ex : ey) * 0.5f;
  float blink = 1.f - std::pow(std::sinf(t * 0.5f), 200.f) * 0.8f;

  // bg
  std::shared_ptr<skity::Shader> bg_gradient;
  {
    std::array<skity::Point, 2> pts = {
        skity::Point{x, y + h * 0.5f, 0.f, 1.f},
        skity::Point{x + w * 0.1f, y + h, 0.f, 1.f},
    };
    std::array<skity::Vec4, 2> colors = {
        skity::Vec4{0.f, 0.f, 0.f, 32.f / 255.f},
        skity::Vec4{0.f, 0.f, 0.f, 16.f / 255.f},
    };

    bg_gradient =
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2);
  }
  bg.setStyle(skity::Paint::kFill_Style);
  bg.setShader(bg_gradient);
  bg.setAntiAlias(true);
  canvas->drawOval(skity::Rect::MakeLTRB(lx + 0.3f - ex, ly + 16.f - ey,
                                         lx + 3.f + ex, ly + 16.f + ey),
                   bg);
  canvas->drawOval(skity::Rect::MakeLTRB(rx + 3.f - ex, ry + 16.f - ey,
                                         rx + 3.f + ex, ry + 16.f + ey),
                   bg);

  {
    std::array<skity::Point, 2> pts = {
        skity::Point{x, y + h * 0.25f, 0.f, 1.f},
        skity::Point{x + w * 0.1f, y + h, 0.f, 1.f},
    };
    std::array<skity::Vec4, 2> colors = {
        skity::Vec4{220.f / 225.f, 220.f / 225.f, 220.f / 225.f, 1.f},
        skity::Vec4{128.f / 255.f, 128.f / 255.f, 128.f / 255.f, 1.f},
    };
    bg_gradient =
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2);
  }

  bg.setShader(bg_gradient);
  canvas->drawOval(skity::Rect::MakeLTRB(lx - ex, ly - ey, lx + ex, ly + ey),
                   bg);
  canvas->drawOval(skity::Rect::MakeLTRB(rx - ex, ry - ey, rx + ex, ry + ey),
                   bg);

  dx = (mx - rx) / (ex * 10.f);
  dy = (my - ry) / (ey * 10.f);
  d = std::sqrtf(dx * dx + dy * dy);
  if (d > 1.f) {
    dx /= d;
    dy /= d;
  }

  dx *= ex * 0.4f;
  dy *= ey * 0.5f;

  bg.setShader(nullptr);
  bg.SetFillColor(32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.f);
  canvas->drawOval(
      skity::Rect::MakeLTRB(
          lx + dx - br, ly + dy + ey * 0.25f * (1 - blink) - br * blink,
          lx + dx + br, ly + dy + ey * 0.25f * (1 - blink) + br * blink),
      bg);

  dx = (mx - rx) / (ex * 10.f);
  dy = (my - ry) / (ey * 10.f);
  d = std::sqrtf(dx * dx + dy * dy);
  if (d > 1.f) {
    dx /= d;
    dy /= d;
  }

  dx *= ex * 0.4f;
  dy *= ey * 0.5f;
  canvas->drawOval(
      skity::Rect::MakeLTRB(
          rx + dx - br, ry + dy + ey * 0.25f * (1 - blink) - br * blink,
          rx + dx + br, ry + dy + ey * 0.25f * (1 - blink) + br * blink),
      bg);

  gloss.setAntiAlias(true);
  gloss.setStyle(skity::Paint::kFill_Style);
  {
    std::array<float, 2> stops = {0.0f, 1.f};
    std::array<skity::Vec4, 2> colors = {
        skity::Vec4{225.f / 225.f, 225.f / 225.f, 225.f / 225.f, 128.f / 255.f},
        skity::Vec4{225.f / 225.f, 225.f / 225.f, 225.f / 225.f, 0.f},
    };

    auto radial = skity::Shader::MakeRadial(
        skity::Point{lx - ex * 0.25f, ly - ey * 0.5f, 0.f, 1.f}, ex * 0.75f,
        colors.data(), stops.data(), 2);
    gloss.setShader(radial);
  }

  canvas->drawOval(skity::Rect::MakeLTRB(lx - ex, ly - ey, lx + ex, ly + ey),
                   gloss);

  {
    std::array<float, 2> stops = {0.0f, 1.f};
    std::array<skity::Vec4, 2> colors = {
        skity::Vec4{225.f / 225.f, 225.f / 225.f, 225.f / 225.f, 128.f / 255.f},
        skity::Vec4{225.f / 225.f, 225.f / 225.f, 225.f / 225.f, 0.f},
    };

    auto radial = skity::Shader::MakeRadial(
        skity::Point{rx - ex * 0.25f, ry - ey * 0.5f, 0.f, 1.f}, ex * 0.75f,
        colors.data(), stops.data(), 2);
    gloss.setShader(radial);
  }
  canvas->drawOval(skity::Rect::MakeLTRB(rx - ex, ry - ey, rx + ex, ry + ey),
                   gloss);
}

void draw_graph(skity::Canvas* canvas, float x, float y, float w, float h,
                float t) {
  std::array<float, 6> samples{};
  std::array<float, 6> sx{};
  std::array<float, 6> sy{};
  float dx = w / 5.f;

  samples[0] =
      (1.f + std::sinf(t * 1.2345f + std::cosf(t * 0.3345f) * 0.44f)) * 0.5f;
  samples[1] =
      (1.f + std::sinf(t * 0.68363f + std::cosf(t * 1.3f) * 1.55f)) * 0.5f;
  samples[2] =
      (1.f + std::sinf(t * 1.1642f + std::cosf(t * 0.33457) * 1.24f)) * 0.5f;
  samples[3] =
      (1.f + std::sinf(t * 0.56345f + std::cosf(t * 1.63f) * 0.14f)) * 0.5f;
  samples[4] =
      (1.f + std::sinf(t * 1.6245f + std::cosf(t * 0.254f) * 0.3f)) * 0.5f;
  samples[5] =
      (1.f + std::sinf(t * 0.345f + std::cosf(t * 0.03f) * 0.6f)) * 0.5f;

  for (int32_t i = 0; i < 6; i++) {
    sx[i] = x + i * dx;
    sy[i] = y + h * samples[i] * 0.8f;
  }

  skity::Paint paint;
  paint.setAntiAlias(true);
  // Graph background
  {
    std::array<skity::Point, 2> pts{
        skity::Point{x, y, 0.f, 1.f},
        skity::Point{x, y + h, 0.f, 1.f},
    };
    std::array<skity::Vec4, 2> colors{
        skity::Vec4{0.f, 160.f / 255.f, 192.f / 255.f, 0.f},
        skity::Vec4{0.f, 160.f / 255.f, 192.f / 255.f, 64.f / 255.f},
    };

    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  }

  skity::Path path;
  path.moveTo(sx[0], sy[0]);
  for (int32_t i = 1; i < 6; i++) {
    path.cubicTo(sx[i - 1] + dx * 0.5f, sy[i - 1], sx[i] - dx * 0.5f, sy[i],
                 sx[i], sy[i]);
  }
  path.lineTo(x + w, y + h);
  path.lineTo(x, y + h);
  path.close();
  canvas->drawPath(path, paint);

  // graph line
  skity::Path graph_line;
  graph_line.moveTo(sx[0], sy[0] + 2.f);
  for (int32_t i = 1; i < 6; i++) {
    graph_line.cubicTo(sx[i - 1] + dx * 0.5f, sy[i - 1] + 2, sx[i] - dx * 0.5f,
                       sy[i] + 2.f, sx[i], sy[i] + 2.f);
  }
  paint.setShader(nullptr);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.SetStrokeColor(0.f, 0.f, 0.f, 32.f / 255.f);
  paint.setStrokeWidth(3.f);

  canvas->drawPath(graph_line, paint);

  skity::Path graph_line2;
  graph_line2.moveTo(sx[0], sy[0]);
  for (int32_t i = 1; i < 6; i++) {
    graph_line2.cubicTo(sx[i - 1] + dx * 0.5f, sy[i - 1], sx[i] - dx * 0.5f,
                        sy[i], sx[i], sy[i]);
  }
  paint.SetStrokeColor(0.f, 160.f / 255.f, 192.f / 255.f, 1.f);
  canvas->drawPath(graph_line2, paint);

  // graph sample pos
  paint.setStyle(skity::Paint::kFill_Style);
  for (int32_t i = 0; i < 6; i++) {
    std::array<skity::Vec4, 2> colors{
        skity::Vec4{0.f, 0.f, 0.f, 32.f / 255.f},
        skity::Vec4{0.f, 0.f, 0.f, 0.f},
    };
    std::array<float, 2> stops = {3.f / 8.f, 1.f};

    auto bg = skity::Shader::MakeRadial({sx[i], sy[i] + 2.f, 0.f, 1.f}, 8.f,
                                        colors.data(), stops.data(), 2);
    paint.setShader(bg);
    canvas->drawRect(
        skity::Rect::MakeXYWH(sx[i] - 10.f, sy[i] - 10.f + 2.f, 20.f, 20.f),
        paint);
  }

  paint.setShader(nullptr);
  paint.SetFillColor(0.f, 160.f / 255.f, 192.f / 255.f, 1.f);
  for (int32_t i = 0; i < 6; i++) {
    canvas->drawCircle(sx[i], sy[i], 4.f, paint);
  }

  paint.SetFillColor(220.f / 255.f, 220.f / 255.f, 220.f / 255.f, 1.f);
  for (int32_t i = 0; i < 6; i++) {
    canvas->drawCircle(sx[i], sy[i], 2.f, paint);
  }
}

void draw_color_wheel(skity::Canvas* canvas, float x, float y, float w, float h,
                      float t) {
  float r0, r1, ax, ay, bx, by, cx, cy, aeps, r;
  float hue = std::sinf(t * 0.12f);

  cx = x + w * 0.5f;
  cy = y + h * 0.5f;
  r1 = (w < h ? w : h) * 0.5f - 5.f;
  r0 = r1 - 20.f;
  aeps = 0.5f / r1;  // half a pixel arc length in radians (2pi cancels out).

  for (int32_t i = 0; i < 6; i++) {
    float a0 = (float)i / 6.f * glm::pi<float>() * 2.f - aeps;
    float a1 = (float)(i + 1.f) / 6.f * glm::pi<float>() * 2.f + aeps;

    float p1_x = cx + std::cos(a0) * r0;
    float p1_y = cy + std::sin(a0) * r0;

    float p3_x = cx + std::cos(a1) * r0;
    float p3_y = cy + std::sin(a1) * r0;

    skity::Vec2 p1r = glm::normalize(skity::Vec2{p1_x - cx, p1_y - cy});
    skity::Vec2 p3r = glm::normalize(skity::Vec2{p3_x - cx, p3_y - cy});
    skity::Vec2 p2r = glm::normalize((p1r + p3r) * 0.5f);
    p2r = skity::Vec2{cx, cy} +
          (r0 + r0 * glm::pi<float>() * 0.1f *
                    std::powf((a1 - a0) * 2.f / glm::pi<float>(), 2)) *
              p2r;

    float p4_x = cx + std::cos(a0) * r1;
    float p4_y = cy + std::sin(a0) * r1;

    float p6_x = cx + std::cos(a1) * r1;
    float p6_y = cy + std::sin(a1) * r1;

    skity::Vec2 p4r = glm::normalize(skity::Vec2{p4_x - cx, p4_y - cy});
    skity::Vec2 p6r = glm::normalize(skity::Vec2{p6_x - cx, p6_y - cy});
    skity::Vec2 p5r = glm::normalize((p6r + p4r) * 0.5f);
    p5r = skity::Vec2{cx, cy} +
          (r1 + r1 * glm::pi<float>() * 0.1f *
                    std::powf((a1 - a0) * 2.f / glm::pi<float>(), 2)) *
              p5r;

    skity::Vec2 p1c = skity::Vec2{p1_x - cx, p1_y - cy};

    skity::Path path;
    path.moveTo(p1_x, p1_y);
    path.quadTo(p2r.x, p2r.y, p3_x, p3_y);
    path.lineTo(p6_x, p6_y);
    path.quadTo(p5r.x, p5r.y, p4_x, p4_y);
    path.close();

    ax = cx + std::cosf(a0) * (r0 + r1) * 0.5f;
    ay = cy + std::sinf(a0) * (r0 + r1) * 0.5f;
    bx = cx + std::cosf(a1) * (r0 + r1) * 0.5f;
    by = cy + std::sinf(a1) * (r0 + r1) * 0.5f;

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kFill_Style);
    paint.setColor(skity::Color_BLUE);
    std::array<skity::Color4f, 2> colors{
        skity::Color4fFromColor(skity::ColorMakeFromHSLA(
            a0 / (glm::pi<float>() * 2.f), 1.f, 0.55f, 255)),
        skity::Color4fFromColor(skity::ColorMakeFromHSLA(
            a1 / (glm::pi<float>() * 2.f), 1.f, 0.55f, 255)),
    };
    std::array<skity::Point, 2> pts{
        skity::Point{ax, ay, 0.f, 1.f},
        skity::Point{bx, by, 0.f, 1.f},
    };
    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
    canvas->drawPath(path, paint);
  }

  {
    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeJoin(skity::Paint::kRound_Join);
    paint.setStrokeWidth(1.f);
    paint.setColor(skity::ColorSetARGB(64, 0, 0, 0));
    // FIXME: path add two circle got error path move and lineto command, need
    // fix
    // skity::Path path; path.addCircle(cx, cy, r0 - 0.5f);
    // path.addCircle(cx, cy, r1 + 0.5f);
    // canvas->drawPath(path, paint);
    canvas->drawCircle(cx, cy, r0 - 0.5f, paint);
    canvas->drawCircle(cx, cy, r1 + 0.5f, paint);
  }

  // selector
  canvas->save();
  canvas->translate(cx, cy);
  canvas->rotate(glm::degrees(hue * glm::pi<float>() * 2.f));

  // Marker on
  {
    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(2.f);
    paint.setColor(skity::ColorSetARGB(192, 255, 255, 255));
    skity::Path path;
    path.addRect(skity::Rect::MakeXYWH(r0 - 1.f, -3.f, r1 - r0 + 2.f, 6.f));
    canvas->drawPath(path, paint);

    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setColor(skity::ColorSetARGB(64, 0, 0, 0));
    paint.setStrokeWidth(1.f);
    canvas->drawRect(
        skity::Rect::MakeXYWH(r0 - 2.f, -4.f, r1 - r0 + 2.f + 2.f, 6.f + 2.f),
        paint);
  }

  // Center triangle
  {
    r = r0 - 6.f;
    ax = std::cosf(120.0f / 180.0f * glm::pi<float>()) * r;
    ay = std::sinf(120.0f / 180.0f * glm::pi<float>()) * r;
    bx = std::cosf(-120.0f / 180.0f * glm::pi<float>()) * r;
    by = std::sinf(-120.0f / 180.0f * glm::pi<float>()) * r;
    skity::Path triangle;
    triangle.moveTo(r, 0.f);
    triangle.lineTo(ax, ay);
    triangle.lineTo(bx, by);
    triangle.close();

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kFill_Style);

    // r, 0, ax,ay, nvgHSLA(hue,1.0f,0.5f,255), nvgRGBA(255,255,255,255)
    std::array<skity::Color4f, 2> colors{
        skity::Color4fFromColor(skity::ColorMakeFromHSLA(hue, 1.0f, 0.5f, 255)),
        skity::Color4fFromColor(skity::ColorSetARGB(255, 255, 255, 255)),
    };
    std::array<skity::Point, 2> pts{
        skity::Point{r, 0.f, 0.f, 1.f},
        skity::Point{ax, ay, 0.f, 1.f},
    };

    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
    canvas->drawPath(triangle, paint);

    // (r+ax)*0.5f,(0+ay)*0.5f, bx,by, nvgRGBA(0,0,0,0), nvgRGBA(0,0,0,255)
    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(0, 0, 0, 0));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(255, 0, 0, 0));
    pts[0].x = (r + ax) * 0.5f;
    pts[0].y = (0 + ay) * 0.5f;
    pts[1].x = bx;
    pts[1].y = by;

    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));

    canvas->drawPath(triangle, paint);

    // Select circle on triangle
    ax = std::cosf(120.0f / 180.0f * glm::pi<float>()) * r * 0.3f;
    ay = std::sinf(120.0f / 180.0f * glm::pi<float>()) * r * 0.4f;
    paint.setStyle(skity::Paint::kStroke_Style);
    paint.setStrokeWidth(2.f);
    skity::Path circle;
    circle.addCircle(ax, ay, 5.f);
    paint.setShader(nullptr);
    paint.setColor(skity::ColorSetARGB(192, 255, 255, 255));
    canvas->drawPath(circle, paint);

    colors[0] = {0, 0, 0, 64.f / 255.f};
    colors[1] = {0.f, 0.f, 0.f, 0.f};

    std::array<float, 2> stops = {7.f / 9.f, 1.f};
    paint.setShader(skity::Shader::MakeRadial(
        skity::Point{ax, ay, 0.f, 1.f}, 9.f, colors.data(), stops.data(), 2));
    skity::Path circle2;
    circle2.addCircle(ax, ay, 8.f);
    canvas->drawPath(circle2, paint);
  }

  canvas->restore();
}

void draw_lines(skity::Canvas* canvas, float x, float y, float w, float h,
                float t) {
  float pad = 5.0f, s = w / 9.0f - pad * 2;
  float pts[4 * 2], fx, fy;
  skity::Paint::Join joins[3] = {skity::Paint::kMiter_Join,
                                 skity::Paint::kRound_Join,
                                 skity::Paint::kBevel_Join};
  skity::Paint::Cap caps[3] = {skity::Paint::kButt_Cap,
                               skity::Paint::kRound_Cap,
                               skity::Paint::kSquare_Cap};

  pts[0] = -s * 0.25f + std::cosf(t * 0.3f) * s * 0.5f;
  pts[1] = std::sinf(t * 0.3f) * s * 0.5f;
  pts[2] = -s * 0.25f;
  pts[3] = 0.f;
  pts[4] = s * 0.25f;
  pts[5] = 0.f;
  pts[6] = s * 0.25f + std::cosf(-t * 0.3f) * s * 0.5f;
  pts[7] = std::sinf(-t * 0.3f) * s * 0.5f;

  for (int32_t i = 0; i < 3; i++) {
    for (int32_t j = 0; j < 3; j++) {
      fx = x + s * 0.5f + (i * 3 + j) / 9.f * w + pad;
      fy = y - s * 0.5f + pad;
      skity::Paint paint;
      paint.setStyle(skity::Paint::kStroke_Style);
      paint.setAntiAlias(true);
      paint.setStrokeCap(caps[i]);
      paint.setStrokeJoin(joins[j]);
      paint.setColor(skity::ColorSetARGB(160, 0, 0, 0));
      paint.setStrokeWidth(s * 0.3f);

      skity::Path path;
      path.moveTo(fx + pts[0], fy + pts[1]);
      path.lineTo(fx + pts[2], fy + pts[3]);
      path.lineTo(fx + pts[4], fy + pts[5]);
      path.lineTo(fx + pts[6], fy + pts[7]);

      canvas->drawPath(path, paint);

      paint.setStrokeCap(skity::Paint::kButt_Cap);
      paint.setStrokeJoin(skity::Paint::kBevel_Join);
      paint.setStrokeWidth(1.f);
      paint.setColor(skity::ColorSetARGB(255, 0, 192, 255));

      canvas->drawPath(path, paint);
    }
  }
}

void draw_widths(skity::Canvas* canvas, float x, float y, float width) {
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setColor(skity::ColorSetARGB(255, 0, 0, 0));
  paint.setStyle(skity::Paint::kStroke_Style);
  for (int i = 0; i < 20; i++) {
    float w = (i + 0.5f) * 0.1f;
    paint.setStrokeWidth(w);
    skity::Path path;
    path.moveTo(x, y);
    path.lineTo(x + width, y + width * 0.3f);
    canvas->drawPath(path, paint);
    y += 10.f;
  }
}

void draw_caps(skity::Canvas* canvas, float x, float y, float width) {
  skity::Paint::Cap caps[3] = {skity::Paint::kButt_Cap,
                               skity::Paint::kRound_Cap,
                               skity::Paint::kSquare_Cap};

  float line_width = 8.f;

  skity::Paint paint;
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(32, 255, 255, 255));
  canvas->drawRect(
      skity::Rect::MakeXYWH(x - line_width / 2.f, y, width + line_width, 40.f),
      paint);
  canvas->drawRect(skity::Rect::MakeXYWH(x, y, width, 40.f), paint);

  paint.setStrokeWidth(line_width);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setColor(skity::Color_BLACK);
  for (int32_t i = 0; i < 3; i++) {
    paint.setStrokeCap(caps[i]);
    skity::Path line;
    line.moveTo(x, y + i * 10.f + 5.f);
    line.lineTo(x + width, y + i * 10.f + 5.f);
    canvas->drawPath(line, paint);
  }
}

void draw_scissor(skity::Canvas* canvas, float x, float y, float t) {
  canvas->save();

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);

  canvas->translate(x, y);
  canvas->rotate(5.f);
  // draw first rect and clip rect for it's area.
  skity::Path rect;
  rect.addRect(skity::Rect::MakeXYWH(-20, -20, 60, 40));
  paint.setColor(skity::ColorSetARGB(255, 255, 0, 0));
  canvas->drawPath(rect, paint);

  // draw second rectangle with offset and rotation.
  canvas->translate(40, 0);
  canvas->rotate(glm::degrees(t));

  paint.setColor(skity::ColorSetARGB(64, 255, 128, 0));
  canvas->drawRect(skity::Rect::MakeXYWH(-20, -10, 60, 30), paint);

  canvas->clipRect(skity::Rect::MakeXYWH(-20, -10, 60, 30));
  canvas->rotate(glm::degrees(t));
  paint.setColor(skity::ColorSetARGB(255, 255, 128, 0));
  canvas->drawRect(skity::Rect::MakeXYWH(-20, -10, 60, 30), paint);

  canvas->restore();
}

void draw_window(skity::Canvas* canvas, const char* title, float x, float y,
                 float w, float h) {
  float corner_radius = 3.f;

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);

  skity::RRect rrect;

  // window
  rrect.setRectXY(skity::Rect::MakeXYWH(x - 5, y - 5, w + 10, h + 10),
                  corner_radius, corner_radius);

  paint.setColor(skity::ColorSetARGB(64, 0, 0, 0));
  canvas->drawRRect(rrect, paint);

  paint.setColor(skity::ColorSetARGB(192, 28, 30, 34));
  rrect.setRectXY(skity::Rect::MakeXYWH(x, y, w, h), corner_radius,
                  corner_radius);
  canvas->drawRRect(rrect, paint);

  // header
  std::array<skity::Color4f, 2> colors{};
  colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(8, 255, 255, 255));
  colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(16, 0, 0, 0));
  std::array<skity::Point, 2> pts{skity::Point{x, y, 0, 1},
                                  skity::Point{x, y + 15, 0.f, 1.f}};
  paint.setShader(
      skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 1, y + 1, w - 2, 30),
                  corner_radius - 1.f, corner_radius - 1.f);
  canvas->drawRRect(rrect, paint);

  paint.setShader(nullptr);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setColor(skity::ColorSetARGB(32, 0, 0, 0));
  skity::Path header;
  header.moveTo(x + 0.5f, y + 0.5f + 30.f);
  header.lineTo(x + 0.5f + w - 1.f, y + 0.5f + 30.f);
  canvas->drawPath(header, paint);

  paint.setTextSize(16.f);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(160, 220, 220, 220));
  canvas->drawSimpleText2(title, x + w / 2.f - 80.f, y + 16 + 2, paint);
}

void draw_search_box(skity::Canvas* canvas, const char* title, float x, float y,
                     float w, float h) {
  float corner_radius = h / 2.f - 1.f;

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);
  // Edit
  skity::RRect rrect;
  rrect.setRectXY(skity::Rect::MakeXYWH(x, y, w, h), corner_radius,
                  corner_radius);
  {
    std::array<skity::Color4f, 2> colors;
    std::array<skity::Point, 2> pts;
    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(16, 0, 0, 0));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(92, 0, 0, 0));
    pts[0] = skity::Point{x, y, 0, 0};
    pts[1] = skity::Point{x, y + h, 0, 0};
    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  }
  canvas->drawRRect(rrect, paint);

  paint.setShader(nullptr);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setColor(skity::ColorSetARGB(48, 0, 0, 0));
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 0.5f, y + 0.5f, w - 1.f, h - 1.f),
                  corner_radius - 0.5f, corner_radius - 0.5f);
  canvas->drawRRect(rrect, paint);

  std::string search_icon = "\uf002";
  paint.setTextSize(h * 0.6f);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(32, 255, 255, 255));
  canvas->drawSimpleText2(search_icon.c_str(), x + h * 0.3f, y + h * 0.8f,
                         paint);

  paint.setTextSize(17.f);
  paint.setColor(skity::ColorSetARGB(32, 255, 255, 255));
  canvas->drawSimpleText2(title, x + h * 1.05f, y + h * 0.5f + 8.f, paint);

  std::string cancle_icon = "\uf2d3";
  paint.setTextSize(h * 0.6f);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(32, 255, 255, 255));
  canvas->drawSimpleText2(cancle_icon.c_str(), x + w - h * 1.0f, y + h * 0.7f,
                         paint);
}

void draw_drop_down(skity::Canvas* canvas, const char* text, float x, float y,
                    float w, float h) {
  float corner_radius = 4.f;
  skity::RRect rrect;
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);
  {
    std::array<skity::Color4f, 2> colors{};
    std::array<skity::Point, 2> pts{};
    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(16, 255, 255, 255));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(16, 0, 0, 0));
    pts[0] = {x, y, 0, 1.f};
    pts[1] = {x, y + h, 0.f, 1.f};
    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  }
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 1.f, y + 1.f, w - 2.f, h - 2.f),
                  corner_radius - 1.f, corner_radius - 1.f);
  canvas->drawRRect(rrect, paint);

  paint.setShader(nullptr);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setColor(skity::ColorSetARGB(48, 0, 0, 0));
  paint.setStrokeWidth(2.f);
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 0.5f, y + 0.5f, w - 1.f, h - 1.f),
                  corner_radius - 0.5f, corner_radius - 0.5f);
  canvas->drawRRect(rrect, paint);

  paint.setStyle(skity::Paint::kFill_Style);
  paint.setTextSize(17.f);
  paint.setColor(skity::ColorSetARGB(160, 255, 255, 255));
  canvas->drawSimpleText2(text, x + h * 0.3f, y + h * 0.7f, paint);

  paint.setTextSize(h * 1.1f);
  std::string angle_right = "\uf105";
  canvas->drawSimpleText2(angle_right.c_str(), x + w - h * 0.8f, y + h * 0.9f,
                         paint);
}

void draw_label(skity::Canvas* canvas, const char* text, float x, float y,
                float w, float h) {
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(128, 255, 255, 255));
  paint.setTextSize(15.f);

  canvas->drawSimpleText2(text, x, y + h * 0.9f, paint);
}

void draw_edit_box_base(skity::Canvas* canvas, float x, float y, float w,
                        float h) {
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(32, 255, 255, 255));
  skity::RRect rrect;
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 1.f, y + 1.f, w - 2.f, h - 2.f),
                  3.f, 3.f);
  canvas->drawRRect(rrect, paint);

  paint.setColor(skity::ColorSetARGB(48, 0, 0, 0));
  paint.setStyle(skity::Paint::kStroke_Style);
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 0.5f, y + 0.5f, w - 1.f, h - 1.f),
                  3.5f, 3.5f);
  canvas->drawRRect(rrect, paint);
}

void draw_edit_box(skity::Canvas* canvas, const char* text, float x, float y,
                   float w, float h) {
  draw_edit_box_base(canvas, x, y, w, h);

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setTextSize(17.f);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(64, 255, 255, 255));
  canvas->drawSimpleText2(text, x + h * 0.3f, y + h * 0.7f, paint);
}

void draw_check_box(skity::Canvas* canvas, const char* text, float x, float y,
                    float w, float h) {
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setTextSize(15.f);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(160, 255, 255, 255));
  canvas->drawSimpleText2(text, x + 28.f, y + h * 0.7f, paint);

  skity::RRect rrect;
  rrect.setRectXY(
      skity::Rect::MakeXYWH(x + 1.f, y + h * 0.5f - 9.f + 1.f, 18.f, 18.f), 3.f,
      3.f);
  paint.setColor(skity::ColorSetARGB(128, 0, 0, 0));
  canvas->drawRRect(rrect, paint);

  paint.setTextSize(20.f);
  paint.setColor(skity::ColorSetARGB(128, 255, 255, 255));
  std::string icon_check = "\uf00c";
  canvas->drawSimpleText2(icon_check.c_str(), x + 1.f, y + h * 0.8f, paint);
}

void draw_button(skity::Canvas* canvas, const char* pre_icon, const char* text,
                 float x, float y, float w, float h, skity::Color col) {
  float corner_radius = 4.f;
  float tw = 0.f, iw = 0.f;

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);
  skity::RRect rrect;
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 1.f, y + 1.f, w - 2.f, h - 2.f),
                  corner_radius - 1.f, corner_radius - 1.f);
  paint.setColor(col);
  canvas->drawRRect(rrect, paint);
  {
    std::array<skity::Color4f, 2> colors{};
    std::array<skity::Point, 2> pts{};
    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(32, 255, 255, 255));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(32, 0, 0, 0));
    pts[0] = {x, y, 0, 1};
    pts[1] = {x, y + h, 0, 1};
    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  }
  canvas->drawRRect(rrect, paint);

  paint.setShader(nullptr);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setColor(skity::ColorSetARGB(48, 0, 0, 0));
  rrect.setRectXY(skity::Rect::MakeXYWH(x + 0.5f, y + 0.5f, w - 1.f, h - 1.f),
                  corner_radius - 0.5f, corner_radius - 0.5f);
  canvas->drawRRect(rrect, paint);

  paint.setStyle(skity::Paint::kFill_Style);
  paint.setTextSize(17.f);
  tw = canvas->simpleTextBounds(text, paint);

  if (pre_icon) {
    paint.setTextSize(h * 0.8f);
    iw = canvas->simpleTextBounds(pre_icon, paint);
    canvas->drawSimpleText2(pre_icon, x + w * 0.5f - tw * 0.5f - iw,
                           y + h * 0.75f, paint);
  }

  paint.setTextSize(17.f);
  paint.setColor(skity::ColorSetARGB(160, 0, 0, 0));
  canvas->drawSimpleText2(text, x + w * 0.5f - tw * 0.5f + iw * 0.25f,
                         y + h * 0.7f - 1.f, paint);
  paint.setColor(skity::ColorSetARGB(160, 255, 255, 255));
  canvas->drawSimpleText2(text, x + w * 0.5f - tw * 0.5f + iw * 0.25f,
                         y + h * 0.7f, paint);
}

void draw_edit_box_num(skity::Canvas* canvas, const char* text,
                       const char* units, float x, float y, float w, float h) {
  float uw;
  draw_edit_box_base(canvas, x, y, w, h);

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setTextSize(15.f);
  paint.setStyle(skity::Paint::kFill_Style);

  uw = canvas->simpleTextBounds(units, paint);

  paint.setColor(skity::ColorSetARGB(64, 255, 255, 255));
  canvas->drawSimpleText2(units, x + w - h * 0.3f - uw, y + h * 0.6f, paint);

  paint.setTextSize(17.f);
  paint.setColor(skity::ColorSetARGB(128, 255, 255, 255));
  float tw = canvas->simpleTextBounds(text, paint);

  canvas->drawSimpleText2(text, x + w - h * 0.5f - uw - tw, y + h * 0.65f,
                         paint);
}

void draw_slider(skity::Canvas* canvas, float pos, float x, float y, float w,
                 float h) {
  float cy = y + (int)(h * 0.5f);
  float kr = (int)(h * 0.25f);
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kFill_Style);
  // Slot
  skity::RRect rrect;
  rrect.setRectXY(skity::Rect::MakeXYWH(x, cy - 2, w, 4), 2, 2);
  paint.setColor(skity::ColorSetARGB(64, 0, 0, 0));
  canvas->drawRRect(rrect, paint);

  // Knob Shadow
  {
    std::array<skity::Color4f, 2> colors{};
    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(64, 0, 0, 0));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(0, 0, 0, 0));
    paint.setShader(skity::Shader::MakeRadial(
        {x + pos * w, cy + 1.f, 0.f, 1.f}, kr + 3, colors.data(), nullptr, 2));
  }
  skity::Path path;
  path.addRect(skity::Rect::MakeXYWH(x + pos * w - kr - 5, cy - kr - 5,
                                     kr * 2 + 5 + 5, kr * 2 + 5 + 5 + 3));
  path.addCircle(x + pos * w, cy, kr, skity::Path::Direction::kCCW);
  canvas->drawPath(path, paint);

  skity::Path knob;
  knob.addCircle(x + pos * w, cy, kr - 1);
  paint.setShader(nullptr);
  paint.setColor(skity::ColorSetARGB(255, 40, 43, 48));
  canvas->drawPath(knob, paint);
  {
    std::array<skity::Color4f, 2> colors{};
    std::array<skity::Point, 2> pts{};
    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(16, 255, 255, 255));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(16, 0, 0, 0));
    pts[0] = {x, cy - kr, 0, 1};
    pts[1] = {x, cy + kr, 0, 1};
    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  }
  canvas->drawPath(knob, paint);

  skity::Path circle;
  circle.addCircle(x + pos * w, cy, kr - 0.5f);
  paint.setShader(nullptr);
  paint.setColor(skity::ColorSetARGB(92, 0, 0, 0));
  paint.setStyle(skity::Paint::kStroke_Style);
  canvas->drawPath(circle, paint);
}

void load_images(std::vector<std::shared_ptr<skity::Pixmap>>& images) {
  for (int32_t i = 0; i < 12; i++) {
    char buffer[128];
    std::sprintf(buffer, "%s/image%d.jpg", EXAMPLE_IMAGE_ROOT, i + 1);
    auto data = skity::Data::MakeFromFileName(buffer);
    if (!data) {
      continue;
    }

    auto codec = skity::Codec::MakeFromData(data);
    if (!codec) {
      continue;
    }

    codec->SetData(data);
    auto pixmap = codec->Decode();
    if (!pixmap) {
      continue;
    }

    images.emplace_back(pixmap);
  }
}

void draw_thumbnails(skity::Canvas* canvas,
                     std::vector<std::shared_ptr<skity::Pixmap>> const& images,
                     float x, float y, float w, float h, float t) {
  float corner_radius = 3.f;
  float ix, iy, iw, ih;
  float thumb = 60.0f;
  float arry = 30.5f;
  int imgw, imgh;
  float stackh = (images.size() / 2.f) * (thumb + 10) + 10;

  float u = (1.f + std::cosf(t * 0.5f)) * 0.5f;
  float u2 = (1.f - std::cosf(t * 0.2f)) * 0.5f;
  float scrollh, dv;

  // Fake shadow
  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setStrokeJoin(skity::Paint::kMiter_Join);
  paint.setColor(skity::ColorSetARGB(64, 0, 0, 0));
  paint.setStrokeWidth(5.f);
  skity::Rect rect{x - 2.5f, y - 2.5f, x + w + 2.5f, y + h + 2.5f};
  canvas->drawRect(rect, paint);

  // window
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setColor(skity::ColorSetARGB(255, 200, 200, 200));
  skity::Path path;
  path.moveTo(x - 10, y + arry);
  path.lineTo(x + 1.f, y + arry - 11.f);
  path.lineTo(x + 1.f, y + arry + 11.f);
  path.close();
  canvas->drawPath(path, paint);
  rect.setXYWH(x, y, w, h);
  canvas->drawRRect(
      skity::RRect::MakeRectXY(rect, corner_radius, corner_radius), paint);

  canvas->save();
  canvas->clipRect(rect);
  canvas->translate(0, -(stackh - h) * u);

  dv = 1.0f / (float)(images.size() - 1);

  for (int32_t i = 0; i < images.size(); i++) {
    float tx, ty, v, a;
    tx = x + 10;
    ty = y + 10;
    tx += (i % 2) * (thumb + 10);
    ty += (i / 2) * (thumb + 10);
    const auto& img = images[i];
    imgw = img->Width();
    imgh = img->Height();
    if (imgw < imgh) {
      iw = thumb;
      ih = iw * (float)imgh / (float)imgw;
      ix = 0;
      iy = -(ih - thumb) * 0.5f;
    } else {
      ih = thumb;
      iw = ih * (float)imgw / (float)imgh;
      ix = -(iw - thumb) * 0.5f;
      iy = 0;
    }

    v = i * dv;
    a = glm::clamp((u2 - v) / dv, 0.f, 1.f);
    if (a < 1.0f) {
      // render is not correct
      draw_spinner(canvas, tx + thumb / 2.f, ty + thumb / 2.f, thumb * 0.25f,
                   t);
    }

    paint.setAlphaF(a);

    skity::Rect image_bounds{};
    image_bounds.setXYWH(tx, ty, thumb, thumb);
    paint.setShader(skity::Shader::MakeShader(img));
    paint.setStyle(skity::Paint::kFill_Style);
    canvas->drawRRect(skity::RRect::MakeRectXY(image_bounds, 5, 5), paint);

    paint.setAlphaF(1.f);
    paint.setShader(nullptr);
    paint.setColor(skity::ColorSetARGB(64, 0, 0, 0));
    paint.setStrokeWidth(2.f);
    paint.setStyle(skity::Paint::kStroke_Style);
    image_bounds.setXYWH(tx - 1.f, ty - 1.f, thumb + 2.f, thumb + 2.f);
    canvas->drawRRect(skity::RRect::MakeRectXY(image_bounds, 6.f, 6.f), paint);
  }

  canvas->restore();

  // Hide fades
  paint.setAlphaF(1.f);
  paint.setStyle(skity::Paint::kFill_Style);
  std::array<skity::Color4f, 2> colors{};
  std::array<skity::Point, 2> pts{};
  colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(255, 200, 200, 200));
  colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(0, 200, 200, 200));
  pts[0] = {x, y, 0, 1};
  pts[1] = {x, y + 6, 0, 1};

  paint.setShader(
      skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  canvas->drawRect(skity::Rect::MakeXYWH(x + 4, y, w - 8, 6), paint);

  pts[0] = {x, y + h, 0, 1};
  pts[1] = {x, y + h - 6, 0, 1};

  paint.setShader(
      skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));

  canvas->drawRect(skity::Rect::MakeXYWH(x + 4, y + h - 6, w - 8, 6), paint);

  // Scroll bar
  paint.setShader(nullptr);
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setStrokeWidth(1.f);
  paint.setColor(skity::ColorSetARGB(62, 0, 0, 0));
  skity::Rect scroll_bar;
  scroll_bar.setXYWH(x + w - 12 - 0.5f, y + 4 - 0.5f, 8 + 1, h - 8 + 1);
  canvas->drawRRect(skity::RRect::MakeRectXY(scroll_bar, 3, 3), paint);

  scrollh = (h / stackh) * (h - 8);
  paint.setColor(skity::ColorSetARGB(255, 220, 220, 220));
  scroll_bar.setXYWH(x + w - 12 + 1, y + 4 + 1 + (h - 8 - scrollh) * u, 8 - 2,
                     scrollh - 2);

  canvas->drawRRect(skity::RRect::MakeRectXY(scroll_bar, 2, 2), paint);
}

void draw_spinner(skity::Canvas* canvas, float cx, float cy, float r, float t) {
  float a0 = 0.f + t * 6;
  float a1 = glm::pi<float>() + t * 6;
  float r0 = r;
  float r1 = r * 0.75f;
  float ax, ay, bx, by;
  float cr = (r0 + r1) * 0.5f;

  skity::Path path;
  path.addCircle(cx, cy, cr);

  skity::Paint paint;
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setStrokeWidth(2.f);
  {
    std::array<skity::Color4f, 2> colors{};
    std::array<skity::Point, 2> pts{};

    ax = cx + std::cosf(a0) * (r0 + r1) * 0.5f;
    ay = cy + std::sinf(a0) * (r0 + r1) * 0.5f;
    bx = cx + std::cosf(a1) * (r0 + r1) * 0.5f;
    by = cy + std::sinf(a1) * (r0 + r1) * 0.5f;

    colors[0] = skity::Color4fFromColor(skity::ColorSetARGB(0, 0, 0, 0));
    colors[1] = skity::Color4fFromColor(skity::ColorSetARGB(128, 0, 0, 0));

    pts[0] = {ax, ay, 0, 1};
    pts[1] = {bx, by, 0, 1};

    paint.setShader(
        skity::Shader::MakeLinear(pts.data(), colors.data(), nullptr, 2));
  }

  canvas->drawPath(path, paint);
}
