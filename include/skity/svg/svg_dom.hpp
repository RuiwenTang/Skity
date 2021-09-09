#ifndef SKITY_SVG_SVG_DOM_HPP
#define SKITY_SVG_SVG_DOM_HPP

#include <memory>
#include <skity/macros.hpp>
#include <string>

namespace skity {

class Canvas;
class Data;

class SVGRoot;

class SK_API SVGDom {
 public:
  ~SVGDom();
  static std::unique_ptr<SVGDom> MakeFromFile(const char* file);
  static std::unique_ptr<SVGDom> MakeFromString(std::string const& content);
  static std::unique_ptr<SVGDom> MakeFromMemory(const char* data, size_t len);
  static std::unique_ptr<SVGDom> MakeFromData(const Data* data);

  void Render(Canvas* canvas);

 private:
  SVGDom(std::shared_ptr<SVGRoot> root);

  std::shared_ptr<SVGRoot> root_;
};

}  // namespace skity

#endif  // SKITY_SVG_SVG_DOM_HPP
