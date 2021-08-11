#ifndef SRC_SKITY_EFFECT_SHADER_BASE_HPP
#define SRC_SKITY_EFFECT_SHADER_BASE_HPP

#include <skity/effect/shader.hpp>
#include <skity/geometry/rect.hpp>

namespace skity {

class GradientShader : public Shader {
 public:
  explicit GradientShader(GradientType type) : Shader(), info_(), type_(type) {}
  ~GradientShader() override = default;

  GradientType asGradient(GradientInfo* info) const override;

  virtual Rect GetFillRect() = 0;

 protected:
  GradientInfo* GetGradientInfo() { return &info_; }
  GradientType GetGradientType() { return type_; }

 private:
  void CopyInfo(GradientInfo* info) const;

 private:
  GradientInfo info_;
  GradientType type_;
};

class LinearGradientShader : public GradientShader {
 public:
  LinearGradientShader(const Point pts[2], const Vec4 colors[],
                       const float pos[], int32_t count);

  ~LinearGradientShader() override = default;

  Rect GetFillRect() override;

 private:
};

}  // namespace skity

#endif  // SRC_SKITY_EFFECT_SHADER_BASE_HPP
