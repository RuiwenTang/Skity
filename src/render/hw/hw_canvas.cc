#include "src/render/hw/hw_canvas.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <skity/codec/pixmap.hpp>

#include "src/render/hw/gl/gl_canvas.hpp"
#include "src/render/hw/hw_mesh.hpp"
#include "src/render/hw/hw_path_raster.hpp"
#include "src/render/hw/hw_pipeline.hpp"

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeHardwareAccelationCanvas(
    uint32_t width, uint32_t height, void* process_loader) {
  // TODO make vk_canvas
  auto mvp = glm::ortho<float>(0, width, height, 0);

  std::unique_ptr<HWCanvas> canvas =
      std::make_unique<GLCanvas>(mvp, width, height);

  canvas->Init(process_loader);

  return canvas;
}

HWCanvas::HWCanvas(Matrix mvp, uint32_t width, uint32_t height)
    : Canvas(),
      mvp_(mvp),
      width_(width),
      height_(height),
      mesh_(std::make_unique<HWMesh>()) {}

HWCanvas::~HWCanvas() = default;

void HWCanvas::Init(void* ctx) { this->OnInit(ctx); }

uint32_t HWCanvas::onGetWidth() const { return width_; }

uint32_t HWCanvas::onGetHeight() const { return height_; }

void HWCanvas::onUpdateViewport(uint32_t width, uint32_t height) {
  mvp_ = glm::ortho(0.f, 0.f, (float)height, (float)width);
  width_ = width;
  height_ = height;
}

HWMesh* HWCanvas::GetMesh() { return mesh_.get(); }

std::unique_ptr<HWDraw> HWCanvas::GenerateOp() {
  auto draw = std::make_unique<HWDraw>(GetPipeline(), state_.HasClip());

  if (state_.MatrixDirty()) {
    draw->SetTransformMatrix(state_.CurrentMatrix());
    state_.ClearMatrixDirty();
  }

  return draw;
}

std::unique_ptr<HWDraw> HWCanvas::GenerateColorOp(Paint const& paint,
                                                  bool stroke,
                                                  Rect const& bounds) {
  auto draw = GenerateOp();
  auto shader = paint.getShader();

  if (shader) {
    auto pixmap = paint.getShader()->asImage();
    Shader::GradientInfo gradient_info{};
    Shader::GradientType gradient_type = shader->asGradient(&gradient_info);
    if (gradient_type != Shader::kNone) {
      if (gradient_type == Shader::kLinear) {
        draw->SetPipelineColorMode(HWPipelineColorMode::kLinearGradient);
        draw->SetGradientBounds(gradient_info.point[0], gradient_info.point[1]);
      } else if (gradient_type == Shader::kRadial) {
        draw->SetGradientBounds(
            gradient_info.point[0],
            {gradient_info.radius[0], gradient_info.radius[1]});
        draw->SetPipelineColorMode(HWPipelineColorMode::kRadialGradient);
      }
      // gradient common info

      draw->SetGradientColors(gradient_info.colors);
      draw->SetGradientPositions(gradient_info.color_offsets);
    } else if (pixmap) {
      auto texture = QueryTexture(pixmap.get());
      draw->SetPipelineColorMode(HWPipelineColorMode::kImageTexture);
      draw->SetTexture(texture);

      float width =
          std::min<float>(bounds.width(), static_cast<float>(pixmap->Width()));
      float height = std::min<float>(bounds.height(),
                                     static_cast<float>(pixmap->Height()));

      glm::vec2 p1{};
      glm::vec2 p2{};
      float x = bounds.left() + (bounds.width() - width) / 2.f;
      float y = bounds.top() + (bounds.height() - height) / 2.f;

      p1.x = x;
      p1.y = y;
      p2.x = x + width;
      p2.y = y + height;

      draw->SetGradientBounds(p1, p2);
    } else {
      // unsupport shader type
    }
  } else {
    draw->SetPipelineColorMode(HWPipelineColorMode::kUniformColor);
    if (stroke) {
      draw->SetUniformColor(paint.GetStrokeColor());
    } else {
      draw->SetUniformColor(paint.GetFillColor());
    }
  }

  return draw;
}

void HWCanvas::onClipRect(const Rect& rect, ClipOp op) {}

void HWCanvas::onClipPath(const Path& path, ClipOp op) {}

void HWCanvas::onDrawLine(float x0, float y0, float x1, float y1,
                          Paint const& paint) {
  HWPathRaster raster(GetMesh(), paint);
  raster.RasterLine({x0, y0}, {x1, y1});
  raster.FlushRaster();

  HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

  auto draw = GenerateColorOp(paint, true, raster.RasterBounds());

  draw->SetStrokeWidth(paint.getStrokeWidth());
  draw->SetColorRange(range);

  draw_ops_.emplace_back(std::move(draw));
}

void HWCanvas::onDrawCircle(float cx, float cy, float radius,
                            Paint const& paint) {
  if (paint.getStyle() != Paint::kFill_Style) {
    Path path;
    path.addCircle(cx, cy, radius);
    onDrawPath(path, paint);

    return;
  }

  HWPathRaster raster{GetMesh(), paint};
  raster.FillCircle(cx, cy, radius);

  raster.FlushRaster();

  HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

  auto draw = GenerateColorOp(paint, false, raster.RasterBounds());
  draw->SetColorRange(range);
  draw->SetStrokeWidth(radius * 2.f);

  draw_ops_.emplace_back(std::move(draw));
}

void HWCanvas::onDrawPath(const Path& path, const Paint& paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  Paint working_paint{paint};
  if (need_fill) {
    working_paint.setStyle(Paint::kFill_Style);

    HWPathRaster raster{GetMesh(), working_paint};

    raster.FillPath(path);
    raster.FlushRaster();

    auto draw = GenerateColorOp(working_paint, false, raster.RasterBounds());

    draw->SetStencilRange(
        {raster.StencilFrontStart(), raster.StencilFrontCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()});
    draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});

    draw_ops_.emplace_back(std::move(draw));
  }

  if (need_stroke) {
    working_paint.setStyle(Paint::kStroke_Style);

    HWPathRaster raster{GetMesh(), working_paint};

    raster.StrokePath(path);
    raster.FlushRaster();

    auto draw = GenerateColorOp(working_paint, true, raster.RasterBounds());

    draw->SetStrokeWidth(paint.getStrokeWidth());

    draw->SetStencilRange(
        {raster.StencilFrontStart(), raster.StencilFrontCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()});
    draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});

    draw_ops_.emplace_back(std::move(draw));
  }
}

void HWCanvas::onDrawRect(Rect const& rect, Paint const& paint) {
  Paint work_paint{paint};
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  if (need_fill) {
    HWPathRaster raster(GetMesh(), paint);
    work_paint.setStyle(Paint::kFill_Style);
    raster.RasterRect(rect);

    raster.FlushRaster();

    HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

    auto draw = GenerateColorOp(work_paint, false, raster.RasterBounds());
    draw->SetColorRange(range);

    draw_ops_.emplace_back(std::move(draw));
  }

  if (need_stroke) {
    HWPathRaster raster(GetMesh(), paint);
    work_paint.setStyle(Paint::kStroke_Style);
    raster.RasterRect(rect);

    raster.FlushRaster();

    HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

    auto draw = GenerateColorOp(work_paint, false, raster.RasterBounds());

    draw->SetStrokeWidth(work_paint.getStrokeWidth());
    draw->SetColorRange(range);

    draw_ops_.emplace_back(std::move(draw));
  }
}

void HWCanvas::onDrawGlyphs(const std::vector<GlyphInfo>& glyphs,
                            const Typeface* typeface, const Paint& paint) {}

void HWCanvas::onSave() { state_.Save(); }

void HWCanvas::onRestore() {
  // step 1 check if there is clip path need clean
  // step 2 restore state
  state_.Restore();
  // step 3 check if there is clip path need to apply
}

void HWCanvas::onTranslate(float dx, float dy) { state_.Translate(dx, dy); }

void HWCanvas::onScale(float sx, float sy) { state_.Scale(sx, sy); }

void HWCanvas::onRotate(float degree) { state_.Rotate(degree); }

void HWCanvas::onRotate(float degree, float px, float py) {
  state_.Rotate(degree, px, py);
}

void HWCanvas::onConcat(const Matrix& matrix) { state_.Concat(matrix); }

void HWCanvas::onFlush() {
  GetPipeline()->Bind();

  mesh_->UploadMesh(GetPipeline());

  // global props set to pipeline
  GetPipeline()->SetViewProjectionMatrix(mvp_);

  for (const auto& op : draw_ops_) {
    op->Draw();
  }

  GetPipeline()->UnBind();

  draw_ops_.clear();
  mesh_->ResetMesh();
}

HWTexture* HWCanvas::QueryTexture(Pixmap* pixmap) {
  auto it = image_texture_store_.find(pixmap);

  if (it != image_texture_store_.end()) {
    return it->second.get();
  }

  auto texture = GenerateTexture();

  texture->Init(HWTexture::Type::kColorTexture, HWTexture::Format::kRGBA);

  texture->Bind();

  texture->Resize(pixmap->Width(), pixmap->Height());
  texture->UploadData(0, 0, pixmap->Width(), pixmap->Height(),
                      (void*)pixmap->Addr());

  texture->UnBind();

  image_texture_store_[pixmap] = std::move(texture);

  return image_texture_store_[pixmap].get();
}

}  // namespace skity