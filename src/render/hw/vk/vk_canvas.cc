#include "src/render/hw/vk/vk_canvas.hpp"

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

#include "src/logging.hpp"
#include "src/render/hw/vk/vk_font_texture.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_render_target.hpp"
#include "src/render/hw/vk/vk_texture.hpp"

namespace skity {

VKCanvas::VKCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
    : HWCanvas(mvp, width, height, density) {}

void VKCanvas::OnInit(GPUContext* ctx) {
  if (ctx->type != GPUBackendType::kVulkan) {
    LOG_ERROR("GPUContext is not vulkan backend");
    assert(false);
    return;
  }

  ctx_ = (GPUVkContext*)ctx;
  vk_phy_features_ = ctx_->GetPhysicalDeviceFeatures();
}

bool VKCanvas::SupportGeometryShader() {
  return vk_phy_features_.geometryShader == VK_TRUE;
}

std::unique_ptr<HWRenderer> VKCanvas::CreateRenderer() {
  auto renderer = std::make_unique<VkRenderer>(ctx_, SupportGeometryShader());
  renderer->Init();

  vk_renderer_ = renderer.get();

  return renderer;
}

std::unique_ptr<HWTexture> VKCanvas::GenerateTexture() {
  return std::make_unique<VKTexture>(vk_renderer_->GetInterface(),
                                     vk_renderer_->Allocator(), vk_renderer_,
                                     ctx_);
}

std::unique_ptr<HWFontTexture> VKCanvas::GenerateFontTexture(
    Typeface* typeface) {
  return std::make_unique<VKFontTexture>(vk_renderer_->GetInterface(), typeface,
                                         vk_renderer_->Allocator(),
                                         vk_renderer_, ctx_);
}

std::unique_ptr<HWRenderTarget> VKCanvas::GenerateBackendRenderTarget(
    uint32_t width, uint32_t height) {
  auto vk_rt = std::make_unique<VKRenderTarget>(
      vk_renderer_->GetInterface(), width, height, vk_renderer_->Allocator(),
      vk_renderer_, ctx_);

  vk_rt->Init();

  return vk_rt;
}

void VKCanvas::onFlush() {
  HandleOrientation();

  HWCanvas::onFlush();
}

void VKCanvas::HandleOrientation() {
  auto ctx_transform = ctx_->GetSurfaceTransform();

  if (ctx_transform == current_transform_) {
    return;
  }

  current_transform_ = ctx_transform;

  glm::mat4 pre_rotate = glm::mat4(1.f);
  glm::vec3 rotation_axis = glm::vec3(0.f, 0.f, 1.f);

  if (current_transform_ & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
    pre_rotate = glm::rotate(pre_rotate, glm::radians(90.f), rotation_axis);
  } else if (current_transform_ & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
    pre_rotate = glm::rotate(pre_rotate, glm::radians(270.f), rotation_axis);
  } else if (current_transform_ & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
    pre_rotate = glm::rotate(pre_rotate, glm::radians(180.f), rotation_axis);
  }

  auto mvp = GetCurrentMVP();

  auto adapt_mvp = pre_rotate * mvp;

  SetCurrentMVP(adapt_mvp);
}

}  // namespace skity