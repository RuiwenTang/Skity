#include <glad/glad.h>
// should after glad.h
#include <GLFW/glfw3.h>

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <skity/effect/path_effect.hpp>
#include <skity/effect/shader.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>

GLFWwindow* init_glfw_window(uint32_t width, uint32_t height,
                             const char* title) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // multisample
  glfwWindowHint(GLFW_SAMPLES, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (window == nullptr) {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    exit(-3);
  }

  return window;
}

static void draw_basic_example(skity::Canvas* canvas) {
  skity::Paint paint;
  paint.setStyle(skity::Paint::kFill_Style);
  paint.setAntiAlias(true);
  paint.setStrokeWidth(4.f);
  paint.SetFillColor(0x42 / 255.f, 0x85 / 255.f, 0xF4 / 255.f, 1.f);

  skity::Rect rect = skity::Rect::MakeXYWH(10, 10, 100, 160);
  canvas->drawRect(rect, paint);

  skity::RRect oval;
  oval.setOval(rect);
  oval.offset(40, 80);
  paint.SetFillColor(0xDB / 255.f, 0x44 / 255.f, 0x37 / 255.f, 1.f);
  canvas->drawRRect(oval, paint);

  paint.SetFillColor(0x0F / 255.f, 0x9D / 255.f, 0x58 / 255.f, 1.f);
  canvas->drawCircle(180, 50, 25, paint);

  rect.offset(80, 50);
  paint.SetStrokeColor(0xF4 / 255.f, 0xB4 / 255.f, 0x0, 1.f);
  paint.setStyle(skity::Paint::kStroke_Style);
  canvas->drawRoundRect(rect, 10, 10, paint);
}

static void draw_path_effect_example(skity::Canvas* canvas) {
  const float R = 115.2f, C = 128.f;
  skity::Path path;
  path.moveTo(C + R, C);
  for (int32_t i = 1; i < 8; i++) {
    float a = 2.6927937f * i;
    path.lineTo(C + R * std::cos(a), C + R * std::sin(a));
  }

  skity::Paint paint;
  paint.setPathEffect(skity::PathEffect::MakeDiscretePathEffect(10.f, 4.f));
  paint.setStyle(skity::Paint::kStroke_Style);
  paint.setStrokeWidth(2.f);
  paint.setAntiAlias(true);
  paint.SetStrokeColor(0x42 / 255.f, 0x85 / 255.f, 0xF4 / 255.f, 1.f);
  canvas->drawPath(path, paint);
}

static void draw_dash_start_example(skity::Canvas* canvas) {
  skity::Path path;
  path.moveTo(199, 34);
  path.lineTo(253, 143);
  path.lineTo(374, 160);
  path.lineTo(287, 244);
  path.lineTo(307, 365);
  path.lineTo(199, 309);
  path.lineTo(97, 365);
  path.lineTo(112, 245);
  path.lineTo(26, 161);
  path.lineTo(146, 143);
  path.close();

  skity::Paint paint;
  paint.setStrokeWidth(3.f);
  paint.setStrokeJoin(skity::Paint::kRound_Join);
  paint.setStrokeCap(skity::Paint::kRound_Cap);
  paint.SetStrokeColor(0, 0, 1, 1);
  paint.SetFillColor(150.f / 255.f, 150.f / 255.f, 1.f, 1.f);
  paint.setAntiAlias(true);
  paint.setStyle(skity::Paint::kStrokeAndFill_Style);
  float pattern[2] = {10.f, 10.f};
  paint.setPathEffect(skity::PathEffect::MakeDashPathEffect(pattern, 2, 0));

  canvas->drawPath(path, paint);
}

void draw_linear_gradient_example(skity::Canvas* canvas) {
  skity::Paint p;
  p.setStyle(skity::Paint::kFill_Style);

  skity::Vec4 colors[] = {
      skity::Vec4{0.f, 1.f, 1.f, 0.f},
      skity::Vec4{0.f, 0.f, 1.f, 1.f},
      skity::Vec4{1.f, 0.f, 0.f, 1.f},
  };
  float positions[] = {0.f, 0.65f, 1.f};

  for (int i = 0; i < 4; i++) {
    canvas->save();
    float blockX = (i % 2) * 100.f;
    float blockY = (i / 2) * 100.f;

    std::vector<skity::Point> pts = {
        skity::Point{blockX, blockY, 0.f, 1.f},
        skity::Point{blockX + 50, blockY + 100, 0.f, 1.f},
    };

    if (i / 2 == 1) {
      canvas->rotate(45.f, blockX, blockY);
    }
    auto lgs = skity::Shader::MakeLinear(pts.data(), colors, positions, 3);
    p.setShader(lgs);
    auto r = skity::Rect::MakeLTRB(blockX, blockY, blockX + 100, blockY + 100);
    canvas->drawRect(r, p);
    canvas->restore();
  }
}

void draw_canvas(skity::Canvas* canvas) {
  draw_basic_example(canvas);

  canvas->save();
  canvas->translate(300, 0);
  draw_path_effect_example(canvas);
  canvas->restore();

  canvas->save();
  canvas->translate(0, 300);
  draw_dash_start_example(canvas);
  canvas->restore();

  canvas->save();
  canvas->translate(400, 300);
  draw_linear_gradient_example(canvas);
  canvas->restore();
}

int main(int argc, const char** argv) {
  GLFWwindow* window = init_glfw_window(800, 800, "SKITY render example");

  glClearColor(1.f, 1.f, 1.f, 1.f);
  glClearStencil(0x0);
  glStencilMask(0xFF);
  // blend is need for anti-alias
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  auto canvas =
      skity::Canvas::MakeGLCanvas(0, 0, 800, 800, (void*)glfwGetProcAddress);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    draw_canvas(canvas.get());

    canvas->flush();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}