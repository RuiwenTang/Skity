#include "src/render/path_raster.hpp"

#include <array>

#include "skity/graphic/path.hpp"
#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/render/path_vertex.hpp"

namespace skity {

template <class COEFF>
void Eval(COEFF coeff, PathVertexBuilder& builder, size_t step,
          size_t start_point_index, size_t& prev_point_index,
          size_t& current_point_index)
{
  for (size_t i = 0; i <= step; i++) {
    prev_point_index = current_point_index;
    current_point_index =
        builder.appendPoint(coeff.evalAt((float)i / (float)step));

    builder.appendTriangle(start_point_index, prev_point_index,
                           current_point_index);
  }
}

std::unique_ptr<PathVertex> PathRaster::rasterPath(Path const& path)
{
  Path::Iter iter{path, false};

  PathVertexBuilder builder;

  std::array<Point, 4> pts;
  Path::Verb verb;

  while ((verb = iter.next(pts.data())) != Path::Verb::kDone) {
    size_t start_point_index = 0;
    size_t prev_point_index = 0;
    size_t current_point_index = 0;
    switch (verb) {
      case Path::Verb::kMove:
        start_point_index = builder.appendPoint(pts[0]);
        prev_point_index = 0;
        break;
      case Path::Verb::kLine:
        prev_point_index = current_point_index;
        current_point_index = builder.appendPoint(pts[1]);
        break;
      case Path::Verb::kQuad:
        Eval(QuadCoeff{pts[0], pts[1], pts[2]}, builder, curve_step,
             start_point_index, prev_point_index, current_point_index);
        continue;
      case Path::Verb::kConic:
        Eval(Conic{pts[0], pts[1], pts[2], iter.conicWeight()}, builder,
             curve_step, start_point_index, prev_point_index,
             current_point_index);
        continue;
      case Path::Verb::kCubic:
        Eval(CubicCoeff{pts}, builder, curve_step, start_point_index,
             prev_point_index, current_point_index);
        continue;
      case Path::Verb::kClose:
        prev_point_index = current_point_index;
        current_point_index = start_point_index;
        break;
      case Path::Verb::kDone:
        break;
    }
    if (verb != Path::Verb::kDone) {
      builder.appendTriangle(start_point_index, prev_point_index,
                             current_point_index);
    }
  }

  return builder.build();
}

}  // namespace skity
