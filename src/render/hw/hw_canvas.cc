#include "src/render/hw/hw_canvas.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/effect/path_effect.hpp>

#include "src/geometry/math.hpp"
#include "src/render/hw/hw_mesh.hpp"
#include "src/render/hw/hw_path_raster.hpp"
#include "src/render/hw/hw_pipeline.hpp"

#ifdef SKITY_OPENGL
#include "src/render/hw/gl/gl_canvas.hpp"
#endif

#ifdef SKITY_VULKAN
#include "src/render/hw/vk/vk_canvas.hpp"
#endif

namespace skity {

std::unique_ptr<Canvas> Canvas::MakeHardwareAccelationCanvas(uint32_t width,
                                                             uint32_t height,
                                                             float density,
                                                             GPUContext* ctx) {
  glm::mat4 mvp;
  if (ctx->type == GPUBackendType::kOpenGL) {
    mvp = glm::ortho<float>(0, width, height, 0);
  } else if (ctx->type == GPUBackendType::kVulkan) {
    mvp = glm::ortho<float>(0, width, 0, height);
  }

  std::unique_ptr<HWCanvas> canvas;
  if (ctx->type == GPUBackendType::kOpenGL) {
#ifdef SKITY_OPENGL
    canvas = std::make_unique<GLCanvas>(mvp, width, height, density);
#endif
#ifdef SKITY_VULKAN
  } else if (ctx->type == GPUBackendType::kVulkan) {
    canvas = std::make_unique<VKCanvas>(mvp, width, height, density);
#endif
  } else {
    // nullptr may cause crash
    return canvas;
  }

  if (canvas) {
    canvas->Init(ctx);
  }

  return canvas;
}

HWCanvas::HWCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : Canvas(),
      mvp_(mvp),
      width_(width),
      height_(height),
      density_(density <= 1.f ? 2.f : density),
      mesh_(std::make_unique<HWMesh>()) {}

HWCanvas::~HWCanvas() {
  for (auto const& it : image_texture_store_) {
    it.second->Destroy();
  }

  for (auto const& it : font_texture_store_) {
    it.second->Destroy();
  }
  GetPipeline()->Destroy();
}

void HWCanvas::Init(GPUContext* ctx) {
  this->OnInit(ctx);
  pipeline_ = CreatePipeline();
}

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
      float x = bounds.left();
      float y = bounds.top();

      p1.x = x;
      p1.y = y;
      p2.x = x + std::max(width, bounds.width());
      p2.y = y + std::max(height, bounds.height());

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

  if (!global_alpha_.IsValid()) {
    global_alpha_.Set(paint.getAlphaF());
    draw->SetGlobalAlpha(paint.getAlphaF());
  } else if (*global_alpha_ != paint.getAlphaF()) {
    global_alpha_.Set(paint.getAlphaF());
    draw->SetGlobalAlpha(paint.getAlphaF());
  }

  return draw;
}

void HWCanvas::onClipPath(const Path& path, ClipOp op) {
  // TODO support other ClipOp
  Paint working_paint;
  working_paint.setStyle(Paint::kFill_Style);

  HWPathRaster raster{GetMesh(), working_paint};

  // raster path as normal path fill
  raster.FillPath(path);

  raster.FlushRaster();

  bool has_clip = state_.HasClip();

  if (full_rect_start_ == -1) {
    // For recursive clip path, need to do full scene check
    // to make stencil buffer right
    // this may cause some performance issues and need optimization
    HWPathRaster tmp_raster{GetMesh(), working_paint};
    tmp_raster.RasterRect(Rect::MakeXYWH(0, 0, width_, height_));

    tmp_raster.FlushRaster();

    full_rect_start_ = tmp_raster.ColorStart();
    full_rect_count_ = tmp_raster.ColorCount();
  }

  if (raster.StencilFrontCount() == 0 && raster.StencilBackCount() == 0) {
    // convex polygon save
    state_.SaveClipPath(
        {raster.ColorStart(), raster.ColorCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()},
        {(uint32_t)full_rect_start_, (uint32_t)full_rect_count_},
        state_.CurrentMatrix());
  } else {
    state_.SaveClipPath(
        {raster.StencilFrontStart(), raster.StencilFrontCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()},
        {(uint32_t)full_rect_start_, (uint32_t)full_rect_count_},
        state_.CurrentMatrix());
  }

  auto draw = std::make_unique<HWDraw>(GetPipeline(), has_clip, true);

  if (raster.StencilBackCount() == 0 && raster.StencilFrontCount() == 0) {
    // this is a convexity polygon

    draw->SetStencilRange({raster.ColorStart(), raster.ColorCount()}, {0, 0});
  } else {
    draw->SetStencilRange(
        {raster.StencilFrontStart(), raster.StencilFrontCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()});
  }

  draw->SetColorRange({(uint32_t)full_rect_start_, (uint32_t)full_rect_count_});

  draw->SetTransformMatrix(state_.CurrentMatrix());

  draw_ops_.emplace_back(std::move(draw));
}

void HWCanvas::onDrawLine(float x0, float y0, float x1, float y1,
                          Paint const& paint) {
  if (paint.getPathEffect()) {
    Canvas::onDrawLine(x0, y0, x1, y1, paint);
    return;
  }

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

  if (FloatNearlyZero(paint.getAlphaF())) {
    return;
  }

  Paint working_paint{paint};
  if (need_fill) {
    working_paint.setStyle(Paint::kFill_Style);

    HWPathRaster raster{GetMesh(), working_paint};
    Path dst;
    if (paint.getPathEffect() &&
        paint.getPathEffect()->filterPath(&dst, path, false, working_paint)) {
      raster.FillPath(dst);
    } else {
      raster.FillPath(path);
    }
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

    Path dst;
    if (paint.getPathEffect() &&
        paint.getPathEffect()->filterPath(&dst, path, true, working_paint)) {
      raster.StrokePath(dst);
    } else {
      raster.StrokePath(path);
    }

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
                            Typeface* typeface, const Paint& paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  float offset_x = 0.f;
  float max_height = 0.f;
  if (need_fill) {
    Paint working_paint{paint};
    working_paint.setStyle(Paint::kFill_Style);

    auto font_texture = QueryFontTexture(typeface);
    HWPathRaster raster{GetMesh(), working_paint};

    for (auto const& info : glyphs) {
      auto region =
          font_texture->GetGlyphRegion(info.id, info.font_size * density_);
      float x = offset_x + info.bearing_x;
      float y = -info.ascent;
      float w = region.z / density_;
      float h = region.w / density_;

      max_height = std::max(h, max_height);

      offset_x += info.advance_x;

      if (h == 0) {
        continue;
      }

      glm::vec4 bounds = {x, y, x + w, y + h};
      glm::vec2 uv_lt = font_texture->CalculateUV(region.x, region.y);
      glm::vec2 uv_rb =
          font_texture->CalculateUV(region.x + region.z, region.y + region.w);

      raster.FillTextRect(bounds, uv_lt, uv_rb);
    }

    raster.FlushRaster();

    auto draw =
        GenerateColorOp(working_paint, false,
                        skity::Rect::MakeXYWH(0.f, 0.f, offset_x, max_height));

    draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});
    draw->SetFontTexture(font_texture->GetHWTexture());

    draw_ops_.emplace_back(std::move(draw));
  }

  offset_x = 0.f;
  max_height = 0.f;

  if (need_stroke) {
    Paint working_paint{paint};
    working_paint.setStyle(Paint::kStroke_Style);

    for (auto const& info : glyphs) {
      HWPathRaster raster{GetMesh(), working_paint};
      glm::mat4 transform =
          glm::translate(glm::identity<glm::mat4>(), {offset_x, 0.f, 0.f});

      Path path = info.path.copyWithMatrix(transform);

      offset_x += info.advance_x;

      if (path.isEmpty()) {
        continue;
      }

      Rect bounds = path.getBounds();
      max_height = std::max(max_height, bounds.height());

      raster.StrokePath(path);
      raster.FlushRaster();

      auto draw = GenerateColorOp(working_paint, true,
                                  Rect::MakeXYWH(0, 0, offset_x, max_height));

      draw->SetStencilRange(
          {raster.StencilFrontStart(), raster.StencilFrontCount()},
          {raster.StencilBackStart(), raster.StencilBackCount()});
      draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});

      draw_ops_.emplace_back(std::move(draw));
    }
  }
}

void HWCanvas::onSave() { state_.Save(); }

void HWCanvas::onRestore() {
  // step 1 check if there is clip path need clean
  if (state_.NeedRevertClipStencil()) {
    auto clip_info = state_.CurrentClipStackValue();
    auto draw = std::make_unique<HWDraw>(GetPipeline(),
                                         false,  // no need to handle clip mask
                                         true);
    draw->SetClearStencilClip(true);
    draw->SetTransformMatrix(clip_info.stack_matrix);

    if (clip_info.bound_range.count > 0) {
      draw->SetColorRange(clip_info.bound_range);
    } else {
      draw->SetStencilRange(clip_info.front_range, clip_info.back_range);
    }

    draw_ops_.emplace_back(std::move(draw));
  }
  // step 2 restore state
  state_.Restore();
  // step 3 check if there is clip path need to apply
  if (state_.NeedDoForwardClip()) {
    auto clip_value = state_.CurrentClipStackValue();
    auto draw = std::make_unique<HWDraw>(GetPipeline(),
                                         false,  // no need to handle clip mask
                                         true);

    draw->SetTransformMatrix(clip_value.stack_matrix);
    if (clip_value.front_range.count == 0 && clip_value.back_range.count == 0) {
      draw->SetColorRange(clip_value.bound_range);
    } else {
      draw->SetStencilRange(clip_value.front_range, clip_value.back_range);
    }

    draw_ops_.emplace_back(std::move(draw));
    state_.ClearForawardClipFlag();
  }
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
  global_alpha_.Reset();
  full_rect_start_ = full_rect_count_ = -1;
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
  // can we move this function call ?
  texture->UnBind();

  image_texture_store_[pixmap] = std::move(texture);

  return image_texture_store_[pixmap].get();
}

HWFontTexture* HWCanvas::QueryFontTexture(Typeface* typeface) {
  auto it = font_texture_store_.find(typeface);
  if (it != font_texture_store_.end()) {
    return it->second.get();
  }

  // generate new one

  auto texture = GenerateFontTexture(typeface);

  texture->Init();

  font_texture_store_[typeface] = std::move(texture);

  return font_texture_store_[typeface].get();
}

}  // namespace skity