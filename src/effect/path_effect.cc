#include <skity/effect/path_effect.hpp>
#include <skity/graphic/path.hpp>

#include "src/effect/discrete_path_effect.hpp"

namespace skity {

bool PathEffect::filterPath(Path* dst, Path const& src, bool stroke) const {
  Path tmp, *tmp_dst = dst;

  if (dst == &src) {
    tmp_dst = &tmp;
  }

  if (this->onFilterPath(tmp_dst, src, stroke)) {
    if (dst == &src) {
      *dst = tmp;
    }

    return true;
  }

  return false;
}

PathEffect::DashType PathEffect::asADash(DashInfo* info) const {
  return this->asADash(info);
}

std::shared_ptr<PathEffect> PathEffect::MakeDiscretePathEffect(
    float seg_length, float dev, uint32_t seed_assist) {
  return std::make_shared<DiscretePathEffect>(seg_length, dev, seed_assist);
}

}  // namespace skity