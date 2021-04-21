#ifndef SKITY_SRC_RENDER_CAP_PROC_HPP
#define SKITY_SRC_RENDER_CAP_PROC_HPP

#include <memory>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class CapProc {
 public:
  CapProc() = default;
  virtual ~CapProc() = default;

  virtual Paint::Cap type() const = 0;
  virtual void proc(Path* path, Point const& pivot, Vector const& normal,
                    Point const& stop, Path* otherPath) = 0;

  static std::unique_ptr<CapProc> MakeCapProc(Paint::Cap cap);
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_CAP_PROC_HPP
