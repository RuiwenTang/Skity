#include "src/render/gl/gl_fill.hpp"

#include <array>

#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLMeshRange GLFill::fillPath(Path const& path, Paint const& paint,
                             GLVertex* gl_vertex) {
  GLMeshRange range{};
  range.front_start = gl_vertex->FrontCount();
  range.front_count = 0;
  range.back_start = gl_vertex->BackCount();
  range.back_count = 0;

  Path::Iter iter(path, true);
  gl_vertex_ = gl_vertex;
  std::array<Point, 4> pts;

  for (;;) {
    Path::Verb verb = iter.next(pts.data());
    switch (verb) {
      case Path::Verb::kMove:
        HandleMoveTo(pts[0]);
        break;
      case Path::Verb::kLine:
        HandleLineTo(pts[0], pts[1]);
        break;
      case Path::Verb::kQuad:
        HandleQuadTo(pts[0], pts[1], pts[2]);
        break;
      case Path::Verb::kConic:
        HandleConicTo(pts[0], pts[1], pts[2], iter.conicWeight());
        break;
      case Path::Verb::kCubic:
        HandleCubicTo(pts[0], pts[1], pts[2], pts[3]);
        break;
      case Path::Verb::kClose:
        HandleClose();
        break;
      case Path::Verb::kDone:
        goto DONE;
        break;
    }
  }

DONE:

  return range;
}

}  // namespace skity