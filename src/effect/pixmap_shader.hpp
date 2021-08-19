#ifndef SKITY_SRC_EFFECT_PIXMAP_SHADER_HPP
#define SKITY_SRC_EFFECT_PIXMAP_SHADER_HPP

#include <skity/effect/shader.hpp>

namespace skity {

class PixmapShader : public Shader {
 public:
  PixmapShader(std::shared_ptr<Pixmap> pixmap);
  ~PixmapShader() override = default;

  std::shared_ptr<Pixmap> asImage() const override;

 private:
  std::shared_ptr<Pixmap> pixmap_;
};

}  // namespace skity

#endif  // SKITY_SRC_EFFECT_PIXMAP_SHADER_HPP
