#include <iostream>
#include <skity/svg/svg_dom.hpp>
#include <string>

int main(int argc, const char** argv) {
  static std::string simple_svg = R"(
<svg version="1.1"
     width="300" height="200"
     xmlns="http://www.w3.org/2000/svg">


  <circle cx="150" cy="100" r="80" fill="green" />

</svg>
)";

  auto dom = skity::SVGDom::MakeFromString(simple_svg);

  if (dom) {
    std::cout << "simple svg parse success" << std::endl;
  } else {
    std::cout << "simple svg parse failed" << std::endl;
  }

  return 0;
}