#include "src/render/hw/hw_canvas.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/render/hw/hw_draw.hpp"
#include "src/render/hw/hw_mesh.hpp"
#include "src/render/hw/hw_pipeline.hpp"

namespace skity {

HWCanvas::HWCanvas(Matrix mvp, uint32_t width, uint32_t height)
    : Canvas(),
      mvp_(mvp),
      width_(width),
      height_(height),
      mesh_(std::make_unique<HWMesh>()) {}

HWCanvas::~HWCanvas() = default;

void HWCanvas::Init() { this->OnInit(); }

uint32_t HWCanvas::onGetWidth() const { return width_; }

uint32_t HWCanvas::onGetHeight() const { return height_; }

void HWCanvas::onUpdateViewport(uint32_t width, uint32_t height) {
  mvp_ = glm::ortho(0.f, 0.f, (float)height, (float)width);
  width_ = width;
  height_ = height;
}

void HWCanvas::onClipRect(const Rect& rect, ClipOp op) {}

void HWCanvas::onClipPath(const Path& path, ClipOp op) {}

void HWCanvas::onDrawLine(float x0, float y0, float x1, float y1,
                          Paint const& paint) {
  // single line just have two line cap
  float stroke_width = glm::min(1.f, paint.getStrokeWidth());
  bool hair_line = stroke_width == 1.f;
  float stroke_radius = stroke_width * 0.5f;

  glm::vec2 p0 = {x0, y0};
  glm::vec2 p1 = {x1, y1};

  glm::vec2 dir = glm::normalize(p1 - p0);
  glm::vec2 normal = glm::vec2{-dir.y, dir.x};

  auto a = p0 + normal * stroke_radius;
  auto b = p0 - normal * stroke_radius;
  auto c = p1 + normal * stroke_radius;
  auto d = p1 - normal * stroke_radius;

  float p_flag = msaa_ ? HW_VERTEX_TYPE_LINE_NORMAL : HW_VERTEX_TYPE_LINE_AA;

  uint32_t a_index = mesh_->AppendVertex(a.x, a.y, p_flag, msaa_ ? 0.f : 1.f);
  uint32_t b_index = mesh_->AppendVertex(b.x, b.y, p_flag, msaa_ ? 0.f : -1.f);
  uint32_t c_index = mesh_->AppendVertex(c.x, c.y, p_flag, msaa_ ? 0.f : 1.f);
  uint32_t d_index = mesh_->AppendVertex(d.x, d.y, p_flag, msaa_ ? 0.f : -1.f);

  std::vector<uint32_t> index_buffer = {};
  index_buffer.emplace_back(a_index);
  index_buffer.emplace_back(b_index);
  index_buffer.emplace_back(d_index);
  index_buffer.emplace_back(a_index);
  index_buffer.emplace_back(c_index);
  index_buffer.emplace_back(d_index);
}

void HWCanvas::onDrawCircle(float cx, float cy, float radius,
                            Paint const& paint) {}

void HWCanvas::onDrawPath(const Path& path, const Paint& paint) {}

void HWCanvas::onDrawOval(Rect const& oval, Paint const& paint) {}

void HWCanvas::onDrawRect(Rect const& rect, Paint const& paint) {}

void HWCanvas::onDrawRRect(RRect const& rrect, Paint const& paint) {}

void HWCanvas::onDrawRoundRect(Rect const& rect, float rx, float ry,
                               Paint const& paint) {}

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

  for (const auto& op : draw_ops_) {
    op->Draw();
  }

  GetPipeline()->UnBind();

  mesh_->ResetMesh();
}

}  // namespace skity