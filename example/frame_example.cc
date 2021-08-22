#include <glad/glad.h>
// should after glad.h
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <glm/glm.hpp>
#include <skity/effect/shader.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

void render_frame_demo(skity::Canvas* canvas, float mx, float my, float width,
                       float height, float t);

void draw_eyes(skity::Canvas* canvas, float x, float y, float w, float h,
               float mx, float my, float t);

void draw_graph(skity::Canvas* canvas, float x, float y, float w, float h,
                float t);

void draw_color_wheel(skity::Canvas* canvas, float x, float y, float w, float h,
                      float t);

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

  double mx, my, t, dt;

  while (!glfwWindowShouldClose(window)) {
    glfwGetCursorPos(window, &mx, &my);

    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    t = glfwGetTime();

    render_frame_demo(canvas.get(), mx, my, width, height, t);

    canvas->flush();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void render_frame_demo(skity::Canvas* canvas, float mx, float my, float width,
                       float height, float t) {
  float x, y, popy;

  draw_eyes(canvas, width - 250, 50, 150, 100, mx, my, t);
  // draw_paragraph
  draw_graph(canvas, 0, height / 2.f, width, height / 2.f, t);
  draw_color_wheel(canvas, width - 300, height - 300, 250.f, 250.f, t);
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
  float blink = 1.f - std::pow(std::sinf(t * 0.5f), 200) * 0.8f;

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
    skity::Path path;
    path.arcTo(cx, cy, r0, r1, a0);
    path.close();

    ax = cx + std::cosf(a0) * (r0 + r1) * 0.5f;
    ay = cy + std::sinf(a0) * (r0 + r1) * 0.5f;
    bx = cx + std::cosf(a1) * (r0 + r1) * 0.5f;
    by = cy + std::sinf(a1) * (r0 + r1) * 0.5f;

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStyle(skity::Paint::kFill_Style);
  }
}
