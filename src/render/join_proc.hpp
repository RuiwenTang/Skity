#ifndef SKITY_SRC_RENDER_JOIN_PROC_HPP
#define SKITY_SRC_RENDER_JOIN_PROC_HPP

#include <memory>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class JoinProc {
 public:
  virtual ~JoinProc() = default;

  virtual Paint::Join type() const = 0;

  virtual void proc(Path* outer, Path* inner, Vector const& beforeUnitNormal,
                    Point const& pivot, Vector const& afterUnitNormal,
                    float radius, float invMiterLimit, bool prevIsLine,
                    bool currIsLine) = 0;

  static std::unique_ptr<JoinProc> MakeJoinProc(Paint::Join join);
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_JOIN_PROC_HPP
