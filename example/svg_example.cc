
#include <cstdlib>
#include <skity/render/canvas.hpp>
#include <skity/svg/svg_dom.hpp>

#include "example_config.hpp"

std::unique_ptr<skity::SVGDom> init_simple_svg() {
  static std::string simple_svg = R"(
<svg width="200px" height="100px" viewBox="0 0 95 50"
     xmlns="http://www.w3.org/2000/svg">
  <g id="g12" stroke="green" fill="#F00" stroke-width="5">
    <circle cx="25" cy="25" r="15" />
    <circle cx="40" cy="25" r="15" />
    <circle cx="55" cy="25" r="15" />
    <circle cx="70" cy="25" r="15" />
 <path d="M 100 100 L 300 100 L 200 300 z"
        fill="red" stroke="blue" stroke-width="3" />
  </g>
</svg>
)";

  //  auto dom = skity::SVGDom::MakeFromString(simple_svg);
  auto dom = skity::SVGDom::MakeFromFile(EXAMPLE_IMAGE_ROOT "/tiger.svg");
  return dom;
}
