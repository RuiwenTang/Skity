#include <skity/effect/path_effect.hpp>
#include <skity/graphic/path.hpp>

#include "src/effect/dash_path_effect.hpp"
#include "src/effect/discrete_path_effect.hpp"

namespace skity {

bool PathEffect::filterPath(Path* dst, Path const& src, bool stroke,
                            Paint const& paint) const {
  Path tmp, *tmp_dst = dst;

  if (dst == &src) {
    tmp_dst = &tmp;
  }

  if (this->onFilterPath(tmp_dst, src, stroke, paint)) {
    if (dst == &src) {
      *dst = tmp;
    }

    return true;
  }

  return false;
}

PathEffect::DashType PathEffect::asADash(DashInfo* info) const {
  return this->onAsADash(info);
}

std::shared_ptr<PathEffect> PathEffect::MakeDiscretePathEffect(
    float seg_length, float dev, uint32_t seed_assist) {
  return std::make_shared<DiscretePathEffect>(seg_length, dev, seed_assist);
}

std::shared_ptr<PathEffect> PathEffect::MakeDashPathEffect(
    const float* intervals, int count, float phase) {
  return std::make_shared<DashPathEffect>(intervals, count, phase);
}

}  // namespace skity