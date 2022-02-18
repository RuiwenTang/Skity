#include "src/render/hw/hw_canvas.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <skity/codec/pixmap.hpp>
#include <skity/config.hpp>
#include <skity/effect/path_effect.hpp>
#include <skity/text/text_blob.hpp>
#include <skity/text/text_run.hpp>

#include "src/geometry/math.hpp"
#include "src/render/hw/hw_mesh.hpp"
#include "src/render/hw/hw_path_raster.hpp"
#include "src/render/hw/hw_renderer.hpp"

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

  render_target_cache_.CleanUp();

  GetPipeline()->Destroy();
}

void HWCanvas::Init(GPUContext* ctx) {
  this->OnInit(ctx);

  renderer_ = CreateRenderer();
  if (draw_list_stack_.empty()) {
    draw_list_stack_.emplace_back(DrawList());
  }
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

  HWPathRaster raster{GetMesh(), working_paint, SupportGeometryShader()};

  // raster path as normal path fill
  raster.FillPath(path);

  raster.FlushRaster();

  bool has_clip = state_.HasClip();

  if (full_rect_start_ == -1) {
    // For recursive clip path, need to do full scene check
    // to make stencil buffer right
    // this may cause some performance issues and need optimization
    HWPathRaster tmp_raster{GetMesh(), working_paint, SupportGeometryShader()};
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

  EnqueueDrawOp(std::move(draw));
}

void HWCanvas::onDrawLine(float x0, float y0, float x1, float y1,
                          Paint const& paint) {
  if (paint.getPathEffect()) {
    Canvas::onDrawLine(x0, y0, x1, y1, paint);
    return;
  }

  HWPathRaster raster(GetMesh(), paint, SupportGeometryShader());
  raster.RasterLine({x0, y0}, {x1, y1});
  raster.FlushRaster();

  HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

  auto draw = GenerateColorOp(paint, true, raster.RasterBounds());

  draw->SetStrokeWidth(paint.getStrokeWidth());
  draw->SetColorRange(range);

  EnqueueDrawOp(std::move(draw), raster.RasterBounds(), paint.getMaskFilter());
}

void HWCanvas::onDrawCircle(float cx, float cy, float radius,
                            Paint const& paint) {
  if (paint.getStyle() != Paint::kFill_Style || SupportGeometryShader()) {
    Path path;
    path.addCircle(cx, cy, radius);
    onDrawPath(path, paint);

    return;
  }

  HWPathRaster raster{GetMesh(), paint, SupportGeometryShader()};
  raster.FillCircle(cx, cy, radius);

  raster.FlushRaster();

  HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

  auto draw = GenerateColorOp(paint, false, raster.RasterBounds());
  draw->SetColorRange(range);
  draw->SetStrokeWidth(radius * 2.f);

  EnqueueDrawOp(std::move(draw), raster.RasterBounds(), paint.getMaskFilter());
}

void HWCanvas::onDrawPath(const Path& path, const Paint& paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  bool stroke_and_fill = need_fill && need_stroke;

  if (FloatNearlyZero(paint.getAlphaF())) {
    return;
  }

  if (stroke_and_fill) {
    PushDrawList();
  }

  Paint working_paint{paint};
  Rect bounds;

  if (need_fill) {
    working_paint.setStyle(Paint::kFill_Style);

    HWPathRaster raster{GetMesh(), working_paint, SupportGeometryShader()};
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

    if (stroke_and_fill) {
      bounds = raster.RasterBounds();
      EnqueueDrawOp(std::move(draw));
    } else {
      EnqueueDrawOp(std::move(draw), raster.RasterBounds(),
                    paint.getMaskFilter());
    }
  }

  if (need_stroke) {
    working_paint.setStyle(Paint::kStroke_Style);

    HWPathRaster raster{GetMesh(), working_paint, SupportGeometryShader()};

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

    if (stroke_and_fill) {
      bounds.join(raster.RasterBounds());
      EnqueueDrawOp(std::move(draw));
    } else {
      EnqueueDrawOp(std::move(draw), raster.RasterBounds(),
                    paint.getMaskFilter());
    }
  }

  if (stroke_and_fill) {
    HandleMaskFilter(PopDrawList(), bounds, paint.getMaskFilter());
  }
}

void HWCanvas::onDrawRect(Rect const& rect, Paint const& paint) {
  if (rect.isEmpty()) {
    return;
  }

  Paint work_paint{paint};
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;
  bool stroke_and_fill = need_fill && need_stroke;
  Rect bounds = rect;

  if (stroke_and_fill) {
    PushDrawList();
  }

  if (need_fill) {
    HWPathRaster raster(GetMesh(), paint, SupportGeometryShader());
    work_paint.setStyle(Paint::kFill_Style);
    raster.RasterRect(rect);

    raster.FlushRaster();

    HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

    auto draw = GenerateColorOp(work_paint, false, raster.RasterBounds());
    draw->SetColorRange(range);

    if (stroke_and_fill) {
      bounds.join(raster.RasterBounds());
      EnqueueDrawOp(std::move(draw));
    } else {
      EnqueueDrawOp(std::move(draw), raster.RasterBounds(),
                    paint.getMaskFilter());
    }
  }

  if (need_stroke) {
    HWPathRaster raster(GetMesh(), paint, SupportGeometryShader());
    work_paint.setStyle(Paint::kStroke_Style);
    raster.RasterRect(rect);

    raster.FlushRaster();

    HWDrawRange range{raster.ColorStart(), raster.ColorCount()};

    auto draw = GenerateColorOp(work_paint, false, raster.RasterBounds());

    draw->SetStrokeWidth(work_paint.getStrokeWidth());
    draw->SetColorRange(range);

    if (stroke_and_fill) {
      bounds.join(raster.RasterBounds());
      EnqueueDrawOp(std::move(draw));
    } else {
      EnqueueDrawOp(std::move(draw), raster.RasterBounds(),
                    paint.getMaskFilter());
    }
  }

  if (stroke_and_fill) {
    HandleMaskFilter(PopDrawList(), bounds, paint.getMaskFilter());
  }
}

void HWCanvas::onDrawBlob(const TextBlob* blob, float x, float y,
                          Paint const& paint) {
  bool need_fill = paint.getStyle() != Paint::kStroke_Style;
  bool need_stroke = paint.getStyle() != Paint::kFill_Style;

  PushDrawList();
  auto blob_size = blob->getBoundSize();
  Rect bounds;
  if (need_stroke) {
    bounds.setXYWH(x - paint.getStrokeWidth(), y - paint.getStrokeWidth(),
                   blob_size.x + paint.getStrokeWidth(),
                   blob_size.y + paint.getStrokeWidth());
  } else {
    float ascent = blob->getBlobAscent();
    bounds.setXYWH(x, y - ascent, blob_size.x, blob_size.y);
  }

  if (need_fill) {
    Paint working_paint{paint};
    working_paint.setStyle(Paint::kFill_Style);

    bool need_path_fill = paint.getTextSize() >= paint.getFontThreshold();
    float offset_x = 0.f;
    for (auto const& run : blob->getTextRun()) {
      if (need_path_fill) {
        offset_x += FillTextRunWithPath(x + offset_x, y, run, working_paint);
      } else {
        offset_x += FillTextRun(x + offset_x, y, run, working_paint);
      }
    }
  }

  if (need_stroke) {
    Paint working_paint{paint};
    working_paint.setStyle(Paint::kStroke_Style);

    float offset_x = 0.f;
    for (auto const& run : blob->getTextRun()) {
      offset_x += StrokeTextRun(x + offset_x, y, run, working_paint);
    }
  }

  HandleMaskFilter(PopDrawList(), bounds, paint.getMaskFilter());
}

void HWCanvas::onSave() { state_.Save(); }

void HWCanvas::onRestore() {
  // step 1 check if there is clip path need clean
  ClearClipMask();
  // step 2 restore state
  state_.Restore();
  // step 3 check if there is clip path need to apply
  ForwardFillClipMask();
}

void HWCanvas::onRestoreToCount(int saveCount) {
  ClearClipMask();

  state_.RestoreToCount(saveCount + 1);
  // step 3 check if there is clip path need to apply
  ForwardFillClipMask();
}

void HWCanvas::onTranslate(float dx, float dy) { state_.Translate(dx, dy); }

void HWCanvas::onScale(float sx, float sy) { state_.Scale(sx, sy); }

void HWCanvas::onRotate(float degree) { state_.Rotate(degree); }

void HWCanvas::onRotate(float degree, float px, float py) {
  state_.Rotate(degree, px, py);
}

void HWCanvas::onConcat(const Matrix& matrix) { state_.Concat(matrix); }

void HWCanvas::onFlush() {
  render_target_cache_.BeginFrame();
  GetPipeline()->Bind();

  mesh_->UploadMesh(GetPipeline());

  // global props set to pipeline
  GetPipeline()->SetViewProjectionMatrix(mvp_);

  for (const auto& op : CurrentDrawList()) {
    op->Draw();
  }

  GetPipeline()->UnBind();

  ClearDrawList();
  mesh_->ResetMesh();
  global_alpha_.Reset();
  full_rect_start_ = full_rect_count_ = -1;
  render_target_cache_.EndFrame();
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

HWRenderTarget* HWCanvas::QueryRenderTarget(Rect const& bounds) {
  uint32_t width = bounds.width();
  uint32_t height = bounds.height();
  auto target = render_target_cache_.QueryTarget(width, height);

  if (target) {
    return target;
  }

  return render_target_cache_.StoreCache(
      GenerateBackendRenderTarget(width, height));
}

float HWCanvas::FillTextRun(float x, float y, TextRun const& run,
                            Paint const& paint) {
  auto typeface = run.lockTypeface();

  if (typeface == nullptr) {
    return 0.f;
  }

  float offset_x = x;
  float max_height = 0.f;

  HWPathRaster raster{GetMesh(), paint, SupportGeometryShader()};

  auto font_texture = QueryFontTexture(typeface.get());

  for (auto const& info : run.getGlyphInfo()) {
    auto region =
        font_texture->GetGlyphRegion(info.id, info.font_size * density_);
    float rx = offset_x + info.bearing_x;
    float ry = y - info.ascent;
    float rw = region.z / density_;
    float rh = region.w / density_;

    max_height = std::max(rh, max_height);

    offset_x += info.advance_x;

    if (rh == 0) {
      continue;
    }

    glm::vec4 bounds = {rx, ry, rx + rw, ry + rh};
    glm::vec2 uv_lt = font_texture->CalculateUV(region.x, region.y);
    glm::vec2 uv_rb =
        font_texture->CalculateUV(region.x + region.z, region.y + region.w);

    raster.FillTextRect(bounds, uv_lt, uv_rb);
  }

  raster.FlushRaster();

  auto draw = GenerateColorOp(
      paint, false, skity::Rect::MakeXYWH(x, y, offset_x, max_height));

  draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});
  draw->SetFontTexture(font_texture->GetHWTexture());

  EnqueueDrawOp(std::move(draw));

  return offset_x - x;
}

float HWCanvas::FillTextRunWithPath(float x, float y, TextRun const& run,
                                    Paint const& paint) {
  auto typeface = run.lockTypeface();

  if (typeface == nullptr) {
    return 0.f;
  }

  float offset_x = x;

  Rect bounds;
  for (auto const& info : run.getGlyphInfo()) {
    Path path = info.path;
    if (path.isEmpty() || info.path_font_size != paint.getTextSize()) {
      path = typeface->getGlyphInfo(info.id, paint.getTextSize(), true).path;
    }

    if (bounds.isEmpty()) {
      bounds = path.getBounds();
    } else {
      bounds.join(path.getBounds());
    }

    glm::mat4 transform =
        glm::translate(glm::identity<glm::mat4>(), {offset_x, y, 0.f});

    HWPathRaster raster{GetMesh(), paint, SupportGeometryShader()};
    raster.FillPath(path.copyWithMatrix(transform));

    raster.FlushRaster();

    auto draw = GenerateColorOp(paint, false, bounds);

    draw->SetStencilRange(
        {raster.StencilFrontStart(), raster.StencilFrontCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()});
    draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});

    EnqueueDrawOp(std::move(draw));

    offset_x += info.advance_x;
  }

  return offset_x - x;
}

float HWCanvas::StrokeTextRun(float x, float y, TextRun const& run,
                              Paint const& paint) {
  auto typeface = run.lockTypeface();

  if (typeface == nullptr) {
    return 0.f;
  }

  float offset_x = x;
  float max_height = 0.f;

  for (auto const& info : run.getGlyphInfo()) {
    HWPathRaster raster{GetMesh(), paint, SupportGeometryShader()};
    glm::mat4 transform =
        glm::translate(glm::identity<glm::mat4>(), {offset_x, y, 0.f});

    Path path = info.path.copyWithMatrix(transform);

    if (path.isEmpty() || info.path_font_size != paint.getTextSize()) {
      // Solve reused TextBlob without path info
      path = typeface->getGlyphInfo(info.id, paint.getTextSize(), true).path;
    }

    if (path.isEmpty()) {
      continue;
    }

    Rect bounds = path.getBounds();
    max_height = std::max(max_height, bounds.height());

    raster.StrokePath(path);
    raster.FlushRaster();

    auto draw = GenerateColorOp(
        paint, true, Rect::MakeXYWH(offset_x, y, info.advance_x, max_height));

    offset_x += info.advance_x;

    draw->SetStrokeWidth(paint.getStrokeWidth());
    draw->SetStencilRange(
        {raster.StencilFrontStart(), raster.StencilFrontCount()},
        {raster.StencilBackStart(), raster.StencilBackCount()});
    draw->SetColorRange({raster.ColorStart(), raster.ColorCount()});

    EnqueueDrawOp(std::move(draw));
  }

  return offset_x - x;
}

void HWCanvas::ClearClipMask() {
  if (!state_.NeedRevertClipStencil()) {
    return;
  }

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

  EnqueueDrawOp(std::move(draw));
}

void HWCanvas::ForwardFillClipMask() {
  state_.ForEachClipStackValue(
      [this](HWCanvasState::ClipStackValue const& clip_value, size_t i) {
        bool has_clip = i != 0;
        auto draw = std::make_unique<HWDraw>(GetPipeline(), has_clip, true);

        draw->SetTransformMatrix(clip_value.stack_matrix);
        draw->SetStencilRange(clip_value.front_range, clip_value.back_range);
        draw->SetColorRange(clip_value.bound_range);

        EnqueueDrawOp(std::move(draw));
      });
}

HWCanvas::DrawList& HWCanvas::CurrentDrawList() {
  return draw_list_stack_.back();
}

void HWCanvas::PushDrawList() { draw_list_stack_.emplace_back(DrawList()); }

HWCanvas::DrawList HWCanvas::PopDrawList() {
  auto draw_list = std::move(draw_list_stack_.back());

  draw_list_stack_.pop_back();

  return draw_list;
}

void HWCanvas::ClearDrawList() {
  while (draw_list_stack_.size() > 1) {
    draw_list_stack_.pop_back();
  }

  draw_list_stack_.back().clear();
}

void HWCanvas::EnqueueDrawOp(std::unique_ptr<HWDraw> draw) {
  CurrentDrawList().emplace_back(std::move(draw));
}

void HWCanvas::EnqueueDrawOp(std::unique_ptr<HWDraw> draw, Rect const& bounds,
                             std::shared_ptr<MaskFilter> const& mask_filter) {
  if (mask_filter) {
    Rect filter_bounds = mask_filter->approximateFilteredBounds(bounds);
    auto fbo = QueryRenderTarget(filter_bounds);

    auto op = std::make_unique<PostProcessDraw>(
        fbo, std::move(draw), filter_bounds, GetPipeline(), state_.HasClip());

    Paint paint;
    paint.setStyle(Paint::kFill_Style);
    HWPathRaster raster{GetMesh(), paint, SupportGeometryShader()};

    raster.RasterRect(filter_bounds);
    raster.FlushRaster();

    op->SetColorRange({raster.ColorStart(), raster.ColorCount()});
    op->SetPipelineColorMode(HWPipelineColorMode::kFBOTexture);
    op->SetGradientBounds({filter_bounds.left(), filter_bounds.top()},
                          {filter_bounds.right(), filter_bounds.bottom()});
    op->SetBlurStyle(mask_filter->blurStyle());
    op->SetBlurRadius(mask_filter->blurRadius());
    op->SetTransformMatrix(state_.CurrentMatrix());
    EnqueueDrawOp(std::move(op));
  } else {
    EnqueueDrawOp(std::move(draw));
  }
}

void HWCanvas::HandleMaskFilter(
    DrawList draw_list, Rect const& bounds,
    std::shared_ptr<MaskFilter> const& mask_filter) {
  if (mask_filter) {
    Rect filter_bounds = mask_filter->approximateFilteredBounds(bounds);

    auto fbo = QueryRenderTarget(filter_bounds);

    auto op = std::make_unique<PostProcessDraw>(fbo, std::move(draw_list),
                                                filter_bounds, GetPipeline(),
                                                state_.HasClip());

    Paint paint;
    paint.setStyle(Paint::kFill_Style);
    HWPathRaster raster{GetMesh(), paint, SupportGeometryShader()};

    raster.RasterRect(filter_bounds);
    raster.FlushRaster();

    op->SetColorRange({raster.ColorStart(), raster.ColorCount()});
    op->SetPipelineColorMode(HWPipelineColorMode::kFBOTexture);
    op->SetGradientBounds({filter_bounds.left(), filter_bounds.top()},
                          {filter_bounds.right(), filter_bounds.bottom()});
    op->SetBlurStyle(mask_filter->blurStyle());
    op->SetBlurRadius(mask_filter->blurRadius());
    op->SetTransformMatrix(state_.CurrentMatrix());
    EnqueueDrawOp(std::move(op));
  } else {
    for (auto& op : draw_list) {
      CurrentDrawList().emplace_back(std::move(op));
    }
  }
}

}  // namespace skity