#ifndef SKITY_SRC_RENDER_STROKE_HPP
#define SKITY_SRC_RENDER_STROKE_HPP

#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {


/**
 * @class Stroke
 *
 * Stroke is a utility class for path stroking
 */
class Stroke {
 public:
  Stroke();
  explicit Stroke(Paint const& paint);

  Paint::Cap getCap() const { return cap_; }
  Paint::Join getJoin() const { return join_; }

  void setCap(Paint::Cap cap) { cap_ = cap; }
  void setJoin(Paint::Join join) { join_ = join; }
  void setMiterLimit(float limit) { miter_limit_ = limit; }
  void setWidth(float width) { width_ = width; }
  void setResScale(float rs) { res_scale_ = rs; }

  void strokePath(Path const& path, Path* result) const;

 private:
  float width_;
  float miter_limit_;
  float res_scale_;
  Paint::Cap cap_;
  Paint::Join join_;
  bool do_fill_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_STROKE_HPP

