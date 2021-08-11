#include <skity/effect/shader.hpp>

namespace skity {

Shader::GradientType Shader::asGradient(GradientInfo *info) const {
  return GradientType::kNone;
}

}  // namespace skity