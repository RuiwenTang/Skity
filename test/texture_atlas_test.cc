#include "src/render/texture_atlas.hpp"

#include <iostream>

void print_region(glm::ivec4 const& region) {
  std::cout << "region = {" << region.x << ", " << region.y << " ," << region.z
            << ", " << region.w << "}" << std::endl;
}

int main(int argc, const char** argv) {
  skity::TextureAtlas texture_atlas{30, 30, 4};

  auto region = texture_atlas.AllocateRegion(20, 20);
  print_region(region);

  auto region2 = texture_atlas.AllocateRegion(20, 15);
  print_region(region2);

  if (region2.y < 0) {
    texture_atlas.Resize(60, 60);
  }

  region2 = texture_atlas.AllocateRegion(20, 15);
  print_region(region2);

  return 0;
}