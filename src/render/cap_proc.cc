#include "src/render/cap_proc.hpp"

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


class RoundCapProc : public CapProc {};

}  // namespace skity
