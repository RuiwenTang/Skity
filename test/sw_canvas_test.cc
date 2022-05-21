#include <skity/codec/codec.hpp>
#include <skity/skity.hpp>
#include <vector>

#include "test/common/test_common.hpp"

class SWCanvasTest : public test::TestApp {
 public:
  SWCanvasTest() : TestApp() {}
  ~SWCanvasTest() override = default;

 protected:
  void OnInit() override {
    InitVertex();
    InitShader();
    InitTexture();

    glClearColor(0.f, 0.f, 0.f, 1.f);
  }

  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT);
    DrawTexture();
  }

 private:
  void InitVertex() {
    std::vector<float> raw_points{
        -0.7f, -0.7f, 0.f, 0.f,  // p0
        -0.7f, 0.7f,  0.f, 1.f,  // p1
        0.7f,  0.7f,  1.f, 1.f,  // p2
        0.7f,  -0.7f, 1.f, 0.f,  // p3
    };

    std::vector<uint32_t> raw_index{
        0, 1, 2, 0, 2, 3,
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, raw_points.size() * sizeof(float),
                 raw_points.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, raw_index.size() * sizeof(uint32_t),
                 raw_index.data(), GL_STATIC_DRAW);
  }
  void InitShader() {
    const char* vs = R"(
        #version 400 core


        layout(location = 0) in vec2 vPosition;
        layout(location = 1) in vec2 vUV;

        out vec2 aUV;
       
        void main() {
            gl_Position = vec4(vPosition, 0.0, 1.0);
            aUV = vUV;
        }
   )";

    const char* fs = R"(
        #version 400 core
        
        uniform sampler2D UserTexture;

        in vec2 aUV;

        out vec4 FragColor;

        void main() {
            vec4 c = texture(UserTexture, aUV);
            FragColor = vec4(c.y, c.z, c.w, c.x);
        }
   )";

    program_ = test::create_shader_program(vs, fs);

    texture_location_ = glGetUniformLocation(program_, "UserTexture");
  }
  void InitTexture() {
    skity::Bitmap bitmap(800, 600);

    for (uint32_t i = 0; i < 800; i++) {
      for (uint32_t j = 0; j < 600; j++) {
        bitmap.setPixel(i, j, skity::Color_WHITE);
      }
    }

    auto canvas = skity::Canvas::MakeSoftwareCanvas(&bitmap);

    DrawCanvas(canvas.get());

    texture_ = test::create_texture(bitmap.getPixelAddr(), bitmap.width(),
                                    bitmap.height());

#ifdef SKITY_HAS_JPEG
    auto jpeg_codec = skity::Codec::MakeJPEGCodec();
    auto jpeg_data = jpeg_codec->Encode(bitmap.getPixmap());

    jpeg_data->WriteToFile("sw_canvas_test.jpg");
#endif
  }

  void DrawTexture() {
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(program_);

    glBindTexture(GL_TEXTURE_2D, texture_);
    glUniform1i(texture_location_, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }

  void DrawCanvas(skity::Canvas* canvas) {
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
    paint.setFillColor(150.f / 255.f, 150.f / 255.f, 1.f, 1.f);
    paint.setStyle(skity::Paint::kFill_Style);

    canvas->drawPath(path, paint);
  }

 private:
  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  GLuint ibo_ = 0;
  GLuint program_ = 0;
  GLuint texture_ = 0;
  GLuint texture_location_ = -1;
};

int main(int argc, const char** argv) {
  SWCanvasTest app;

  app.Start();
  return 0;
}
