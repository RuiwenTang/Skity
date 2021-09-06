
#ifndef SKITY_SRC_SVG_TYPES_HPP
#define SKITY_SRC_SVG_TYPES_HPP

#include <cassert>
#include <skity/geometry/point.hpp>
#include <skity/geometry/rect.hpp>
#include <skity/graphic/color.hpp>
#include <string>
#include <vector>

#include "src/utils/lazy.hpp"

namespace skity {

using SVGColorType = Color;
using SVGIntegerType = int32_t;
using SVGNumberType = float;
using SVGStringType = std::string;
using SVGViewBoxType = Rect;
using SVGTransformType = Matrix;
using SVGPointsType = std::vector<Point>;

enum class SVGPropertyState {
  kUnspecified,
  kInherit,
  kValue,
};

// https://www.w3.org/TR/SVG11/intro.html#TermProperty
template <typename T, bool kInheritable>
class SVGProperty {
 public:
  using ValueT = T;

  SVGProperty() : state_(SVGPropertyState::kUnspecified) {}
  explicit SVGProperty(SVGPropertyState state) : state_(state) {}
  explicit SVGProperty(T const& value) : state_(SVGPropertyState::kValue) {
    value_.Set(value);
  }
  ~SVGProperty() = default;

  template <typename... Args>
  void Init(Args&&... args) {
    state_ = SVGPropertyState::kValue;
    value_.template Init(std::forward<Args>(args)...);
  }

  bool IsInheritable() const { return kInheritable; }
  bool IsValue() const { return state_ == SVGPropertyState::kValue; }
  T* GetMaybeNull() const { return value_.GetMaybeNull(); }

  void Set(SVGPropertyState state) {
    state_ = state;
    if (state_ != SVGPropertyState::kValue) {
      value_.Reset();
    }
  }

  void Set(T const& value) {
    state_ = SVGPropertyState::kValue;
    value_.Set(value);
  }

  T* operator->() const {
    assert(state_ == SVGPropertyState::kValue);
    assert(!value_.empty());
    return value_.get();
  }

  T& operator*() const {
    assert(state_ == SVGPropertyState::kValue);
    assert(!value_.empty());
    return *value_;
  }

 private:
  SVGPropertyState state_;
  Lazy<T> value_;
};

class SVGLength {
 public:
  enum class Unit {
    kUnknown,
    kNumber,
    kPercentage,
    kEMS,
    kEXS,
    kPX,
    kCM,
    kMM,
    kIN,
    kPT,
    kPC,
  };

  SVGLength() : value_(0.f), unit_(Unit::kUnknown) {}
  explicit SVGLength(float v, Unit u = Unit::kNumber) : value_(v), unit_(u) {}

  bool operator==(SVGLength const& other) const {
    return unit_ == other.unit_ && value_ == other.value_;
  }

  bool operator!=(SVGLength const& other) const { return !(*this == other); }

  const float& Value() const { return value_; }
  const Unit& unit() const { return unit_; }

 private:
  float value_;
  Unit unit_;
};

class SVGColor {
 public:
  enum class Type {
    kCurrentColor,
    kColor,
    kICCColor,
  };

  SVGColor() : type_(Type::kColor), color_(Color_BLACK) {}
  explicit SVGColor(Type type) : type_(type), color_(Color_BLACK) {}
  explicit SVGColor(SVGColorType const& c) : type_(Type::kColor), color_(c) {}

  bool operator==(SVGColor const& other) const {
    return type_ == other.type_ && color_ == other.color_;
  }
  bool operator!=(SVGColor const& other) const { return !(*this == other); }

  Type type() const { return type_; }
  const SVGColorType& Color() const { return color_; }

 private:
  Type type_;
  SVGColorType color_;
};

class SVGPaint {
 public:
  enum class Type {
    kNone,
    kColor,
    kIRI,
  };

  SVGPaint() : type_(Type::kNone), color_(Color_BLACK) {}
  explicit SVGPaint(Type t) : type_(t), color_(Color_BLACK) {}
  explicit SVGPaint(SVGColor const& color)
      : type_(Type::kColor), color_(color) {}

  bool operator==(SVGPaint const& other) const {
    return type_ == other.type_ && color_ == other.color_;
  }

  bool operator!=(SVGPaint const& other) const { return !(*this == other); }

  Type type() const { return type_; }

  SVGColor const& color() const { return color_; }

 private:
  Type type_;
  SVGColor color_;
  // SVGIRI iri;
};

enum class SVGLineCap {
  kButt,
  kRound,
  kSquare,
};

class SVGLineJoin {
 public:
  enum class Type {
    kMiter,
    kRound,
    kBevel,
    kInherit,
  };

  SVGLineJoin() : type_(Type::kInherit) {}
  explicit SVGLineJoin(Type t) : type_(t) {}

 private:
  Type type_;
};

class SVGVisibility {
 public:
 private:
};

class SVGDashArray {
 public:
  enum class Type {
    kNone,
    kDashArray,
    kInherit,
  };

  SVGDashArray() : type_(Type::kNone) {}
  explicit SVGDashArray(Type t) : type_(t) {}
  explicit SVGDashArray(std::vector<SVGLength>&& dash_array)
      : type_(Type::kDashArray), dash_array_(std::move(dash_array)) {}

  bool operator==(SVGDashArray const& other) {
    return type_ == other.type_ && dash_array_ == other.dash_array_;
  }

  bool operator!=(SVGDashArray const& other) { return !(*this == other); }

  Type type() const { return type_; }

  std::vector<SVGLength> const& DashArray() const { return dash_array_; }

 private:
  Type type_;
  std::vector<SVGLength> dash_array_;
};

struct SVGPreserveAspectRatio {
  enum Align : uint8_t {
    // These values are chosen such that bits [0,1] encode X alignment, and
    // bits [2,3] encode Y alignment.
    kXMinYMin = 0x00,
    kXMidYMin = 0x01,
    kXMaxYMin = 0x02,
    kXMinYMid = 0x04,
    kXMidYMid = 0x05,
    kXMaxYMid = 0x06,
    kXMinYMax = 0x08,
    kXMidYMax = 0x09,
    kXMaxYMax = 0x0a,

    kNone = 0x10,
  };

  enum Scale {
    kMeet,
    kSlice,
  };

  Align align = kXMidYMid;
  Scale scale = kMeet;
};

}  // namespace skity

#endif  // SKITY_SRC_SVG_TYPES_HPP
