#include "src/render/cap_proc.hpp"

#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

class ButtCapProc : public CapProc {
 public:
  ButtCapProc() = default;
  ~ButtCapProc() override = default;

  Paint::Cap type() const override { return Paint::Cap::kButt_Cap; }

  void proc(Path* paht, Point const& pivot, Vector const& normal,
            Point const& stop, Path* otherPath) override
  {
    paht->lineTo(stop.x, stop.y);
  }
};

class RoundCapProc : public CapProc {
 public:
  RoundCapProc() = default;
  ~RoundCapProc() override = default;

  Paint::Cap type() const override { return Paint::Cap::kRound_Cap; }

  void proc(Path* path, Point const& pivot, Vector const& normal,
            Point const& stop, Path* otherPath) override
  {
    Vector parallel;
    PointRotateCW(normal, std::addressof(parallel));

    Point projected_center = pivot + parallel;

    path->conicTo(projected_center + normal, projected_center, FloatRoot2Over2);
    path->conicTo(projected_center - normal, stop, FloatRoot2Over2);
  }
};

}  // namespace skity
