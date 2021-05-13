#include "src/render/draw.hpp"

namespace skity {

void DrawOp::prepare() {
  if (!prepared_) {
    this->onPrepare();
    prepared_ = true;
  }
}

}  // namespace skity