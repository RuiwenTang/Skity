#ifndef SKITY_SRC_EFFECT_DASH_PATH_EFFECT_HPP
#define SKITY_SRC_EFFECT_DASH_PATH_EFFECT_HPP

#include <memory>
#include <skity/effect/path_effect.hpp>

namespace skity {

class DashPathEffect : public PathEffect {
 public:
  DashPathEffect(const float intervals[], int32_t count, float phase);

  ~DashPathEffect() override = default;

 protected:
  bool onFilterPath(Path *, const Path &, bool, Paint const &) const override;

  DashType onAsADash(DashInfo *) const override;

 private:
  /**
   * Update internal phase_, initial_dash_length_, initial_dash_index_,
   * internal_length_
   * @param phase
   */
  void CalcDashParameters(float phase);

 private:
  std::unique_ptr<float[]> intervals_;
  int32_t count_ = 0;
  float phase_;

  float initial_dash_length_;
  int32_t initial_dash_index_;
  float interval_length_;
};

}  // namespace skity

#endif  // SKITY_SRC_EFFECT_DASH_PATH_EFFECT_HPP
