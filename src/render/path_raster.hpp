
#include <memory>

#include "src/render/path_vertex.hpp"

namespace skity {

class Path;

class PathRaster {
 public:
  explicit PathRaster(size_t curve_step = 10) : curve_step(curve_step) {}
  ~PathRaster() = default;

  std::unique_ptr<PathVertex> rasterPath(Path const& path);

 private:
  size_t curve_step;
};

}  // namespace skity
