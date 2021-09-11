
#include "src/svg/svg_attribute_parser.hpp"

#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>

#include "src/graphic/color_parser.hpp"

namespace skity {

static bool is_between(char c, char min, char max) {
  return (unsigned)(c - min) <= (unsigned)(max - min);
}

static bool is_eos(char c) { return !c; }

static bool is_ws(char c) { return is_between(c, 1, 32); }

static bool is_sep(char c) { return is_ws(c) || c == ',' || c == ';'; }

static bool is_digit(char c) { return is_between(c, '0', '9'); }

static const char *skip_ws(const char *str) {
  while (is_ws(*str)) {
    str++;
  }
  return str;
}

static const char *find_scalar(const char *str, float *value) {
  str = skip_ws(str);

  char *stop;
  float v = std::strtof(str, &stop);
  if (str == stop) {
    return nullptr;
  }

  if (value) {
    *value = v;
  }
  return stop;
}

static int to_hex(char c) {
  if (is_digit(c)) {
    return c = '0';
  }

  c |= 0x20;
  if (is_between(c, 'a', 'f')) {
    return c + 10 - 'a';
  } else {
    return -1;
  }
}

static bool is_hex(char c) { return to_hex(c) >= 0; }

static const char *find_hex(const char *str, uint32_t *value) {
  str = skip_ws(str);
  if (!is_hex(*str)) {
    return nullptr;
  }

  uint32_t n = 0;
  int max_digits = 8;
  int digit;
  while ((digit = to_hex(*str)) >= 0) {
    if (--max_digits < 0) {
      return nullptr;
    }
    n = (n << 4) | digit;
    str += 1;
  }
  if (*str == 0 || is_ws(*str)) {
    if (value) {
      *value = n;
      return str;
    }
  }
  return nullptr;
}

const char *find_s32(const char *str, int32_t *value) {
  str = skip_ws(str);
  int sign = 0;
  if (*str == '-') {
    sign = -1;
    str += 1;
  }
  if (!is_digit(*str)) {
    return nullptr;
  }

  int n = 0;
  while (is_digit(*str)) {
    n = 10 * n + *str - '0';
    str += 1;
  }
  if (value) {
    *value = (n ^ sign) - sign;
  }
  return str;
}

static uint32_t nib2byte(uint32_t n) { return (n << 4) | n; }

static const char *find_color(const char *str, Color *color) {
  unsigned int old_alpha = ColorGetA(*color);
  if (str[0] == '#') {
    uint32_t hex;
    const char *end = find_hex(str + 1, &hex);
    if (end == nullptr) {
      return end;
    }
    size_t len = end - str - 1;
    if (len == 3 || len == 4) {
      uint32_t a = len == 4 ? nib2byte(hex >> 12) : old_alpha;
      uint32_t r = nib2byte((hex >> 8) & 0xF);
      uint32_t g = nib2byte((hex >> 4) & 0xF);
      uint32_t b = nib2byte((hex >> 0) & 0xF);
      *color = ColorSetARGB(a, r, g, b);
      return end;
    } else if (len == 6 || len == 8) {
      if (len == 6) {
        hex |= old_alpha << 24;
      }
      *color = hex;
      return end;
    } else {
      return nullptr;
    }
  } else {
    return ColorParser::FindNamedColor(str, std::strlen(str), color);
  }
}
SVGAttributeParser::SVGAttributeParser(const char *string) : cur_pos(string) {}

template <class F>
bool SVGAttributeParser::AdvanceWhile(F func) {
  auto initial = cur_pos;
  while (func(*cur_pos)) {
    cur_pos++;
  }
  return cur_pos != initial;
}

bool SVGAttributeParser::MatchStringToken(const char *token,
                                          const char **new_pos) const {
  const char *c = cur_pos;
  while (*c && *token && *c == *token) {
    c++;
    token++;
  }

  if (*token) {
    return false;
  }

  if (new_pos) {
    *new_pos = c;
  }

  return true;
}

bool SVGAttributeParser::ParseEOSToken() { return is_eos(*cur_pos); }

bool SVGAttributeParser::ParseSepToken() { return this->AdvanceWhile(is_sep); }

bool SVGAttributeParser::ParseWSToken() { return this->AdvanceWhile(is_ws); }

bool SVGAttributeParser::ParseCommaWspToken() {
  return this->ParseWSToken() || this->ParseExpectedStringToken(",");
}

bool SVGAttributeParser::ParseExpectedStringToken(const char *expected) {
  const char *new_pos;
  if (!MatchStringToken(expected, &new_pos)) {
    return false;
  }

  cur_pos = new_pos;
  return true;
}

bool SVGAttributeParser::ParseScalarToken(float *value) {
  if (const char *next = find_scalar(cur_pos, value)) {
    cur_pos = next;
    return true;
  }
  return false;
}

bool SVGAttributeParser::ParseInt32Token(int32_t *value) {
  if (const char *next = find_s32(cur_pos, value)) {
    cur_pos = next;
    return true;
  }
  return false;
}

bool SVGAttributeParser::ParseHexToken(uint32_t *value) {
  if (const char *next = find_hex(cur_pos, value)) {
    cur_pos = next;
    return true;
  }
  return false;
}

bool SVGAttributeParser::ParseLengthUnitToken(SVGLength::Unit *value) {
  static const struct {
    const char *unit_name;
    SVGLength::Unit unit;
  } g_unit_info[] = {
      {"%", SVGLength::Unit::kPercentage}, {"em", SVGLength::Unit::kEMS},
      {"ex", SVGLength::Unit::kEXS},       {"px", SVGLength::Unit::kPX},
      {"cm", SVGLength::Unit::kCM},        {"mm", SVGLength::Unit::kMM},
      {"in", SVGLength::Unit::kIN},        {"pt", SVGLength::Unit::kPT},
      {"pc", SVGLength::Unit::kPC},
  };

  for (size_t i = 0; i < 9; i++) {
    if (this->ParseExpectedStringToken(g_unit_info[i].unit_name)) {
      *value = g_unit_info[i].unit;
      return true;
    }
  }
  return false;
}

bool SVGAttributeParser::ParseNamedColorToken(Color *c) {
  if (const char *next =
          ColorParser::FindNamedColor(cur_pos, std::strlen(cur_pos), c)) {
    cur_pos = next;
    return true;
  }
  return false;
}

bool SVGAttributeParser::ParseHexColorToken(Color *c) {
  uint32_t v;
  const char *initial = cur_pos;

  if (!this->ParseExpectedStringToken("#") || !this->ParseHexToken(&v)) {
    return false;
  }

  switch (cur_pos - initial) {
    case 7:
      // matched #xxxxxx
      break;
    case 4:
      // matched #xxx;
      v = ((v << 12) & 0x00f00000) | ((v << 8) & 0x000ff000) |
          ((v << 4) & 0x00000ff0) | ((v << 0) & 0x0000000f);
      break;
    default:
      return false;
  }

  *c = v | 0xFF000000;
  return true;
}

bool SVGAttributeParser::ParseColorComponentToken(int32_t *c) {
  const auto parse_internal = [this](int32_t *v) -> bool {
    const char *p = find_s32(cur_pos, v);
    if (!p || *p == '.') {
      return false;
    }

    if (*p == '%') {
      *v = std::round(*v * 255.f / 100.f);
      p++;
    }

    cur_pos = p;
    return true;
  };

  const auto parse_fractional = [this](int32_t *v) -> bool {
    float s;
    const char *p = find_scalar(cur_pos, &s);
    if (!p || *p != '%') {
      return false;
    }
    p++;
    *v = std::round(s * 255.f / 100.f);
    cur_pos = p;
    return true;
  };

  if (!parse_internal(c) && !parse_fractional(c)) {
    return false;
  }

  *c = glm::clamp(*c, 0, 255);
  return true;
}

bool SVGAttributeParser::ParseRGBColorToken(Color *c) {
  return this->ParseParenthesized(
      "rgb",
      [this](Color *c) -> bool {
        int32_t r, g, b;
        if (this->ParseColorComponentToken(&r) && this->ParseSepToken() &&
            this->ParseColorComponentToken(&g) && this->ParseSepToken() &&
            this->ParseColorComponentToken(&b)) {
          *c = ColorSetRGB(static_cast<uint8_t>(r), static_cast<uint8_t>(g),
                           static_cast<uint8_t>(b));
          return true;
        }
        return false;
      },
      c);
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeColor
// And https://www.w3.org/TR/CSS2/syndata.html#color-units
template <>
bool SVGAttributeParser::Parse(SVGColorType *color) {
  Color c;
  this->ParseWSToken();

  bool parsed_value = false;
  if (this->ParseHexColorToken(&c) || this->ParseNamedColorToken(&c) ||
      this->ParseRGBColorToken(&c)) {
    *color = SVGColorType(c);
    parsed_value = true;
    this->ParseWSToken();
  }

  return parsed_value && this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/types.html#InterfaceSVGColor
template <>
bool SVGAttributeParser::Parse(SVGColor *color) {
  SVGColorType c;
  bool parsed_value = false;
  if (this->Parse(&c)) {
    *color = SVGColor(c);
    parsed_value = true;
  } else if (this->ParseExpectedStringToken("currentColor")) {
    *color = SVGColor(SVGColor::Type::kCurrentColor);
    parsed_value = true;
  }
  return parsed_value && this->ParseEOSToken();
}

bool SVGAttributeParser::ParseFuncIRI(void *) { return false; }

bool SVGAttributeParser::ParseMatrixToken(Matrix *matrix) {
  return this->ParseParenthesized(
      "matrix",
      [this](Matrix *m) -> bool {
        float scalars[6];
        for (int32_t i = 0; i < 6; i++) {
          if (!(this->ParseScalarToken(scalars + i) &&
                (i > 4 || this->ParseSepToken()))) {
            return false;
          }
        }

        /**
         * matrix(a, b, c, d, e, f)
         * [ a c e]      [a c 0 e]
         * [ b d f] =>   [b d 0 f]
         * [ 0 0 1]      [0 0 1 0]
         *               [0 0 0 1]
         */

        float a = scalars[0];
        float b = scalars[1];
        float c = scalars[2];
        float d = scalars[3];
        float e = scalars[4];
        float f = scalars[5];
        m->operator[](0) = {a, c, 0, e};
        m->operator[](1) = {b, d, 0, f};
        m->operator[](2) = {0, 0, 1.f, 0};
        m->operator[](3) = {0, 0, 0, 1.f};

        return true;
      },
      matrix);
}

bool SVGAttributeParser::ParseTranslateToken(Matrix *matrix) {
  return this->ParseParenthesized(
      "translate",
      [this](Matrix *m) -> bool {
        float tx = 0.f;
        float ty = 0.f;
        this->ParseWSToken();
        if (!this->ParseScalarToken(&tx)) {
          return false;
        }

        if (!this->ParseSepToken() || !this->ParseScalarToken(&ty)) {
          ty = 0.f;
        }
        *m = glm::translate(glm::identity<Matrix>(), {tx, ty, 0.f});
        return true;
      },
      matrix);
}

bool SVGAttributeParser::ParseScaleToken(Matrix *matrix) {
  return this->ParseParenthesized(
      "translate",
      [this](Matrix *m) -> bool {
        float sx = 0.f;
        float sy = 0.f;
        this->ParseWSToken();
        if (!this->ParseScalarToken(&sx)) {
          return false;
        }

        if (!this->ParseSepToken() || !this->ParseScalarToken(&sy)) {
          sy = sx;
        }
        *m = glm::scale(glm::identity<Matrix>(), {sx, sy, 1.f});
        return true;
      },
      matrix);
}

bool SVGAttributeParser::ParseRotateToken(Matrix *matrix) {
  return this->ParseParenthesized(
      "rotate",
      [this](Matrix *m) -> bool {
        float angle;
        if (!this->ParseScalarToken(&angle)) {
          return false;
        }

        float cx = 0.f;
        float cy = 0.f;
        // optional [<cx> <cy>]
        if (this->ParseSepToken() && this->ParseScalarToken(&cx)) {
          if (!(this->ParseSepToken() && this->ParseScalarToken(&cy))) {
            return false;
          }
        }

        *m = glm::rotate(glm::identity<Matrix>(), angle, {0.f, 0.f, 1.f});
        return true;
      },
      matrix);
}

bool SVGAttributeParser::ParseSkewXToken(Matrix *) {
  // TODO implement
  return false;
}

bool SVGAttributeParser::ParseSkewYToken(Matrix *) {
  // TODO implement
  return false;
}

template <>
bool SVGAttributeParser::Parse(SVGStringType *result) {
  if (this->ParseEOSToken()) {
    return false;
  }

  *result = SVGStringType(cur_pos);
  cur_pos += result->size();
  return this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeNumber
template <>
bool SVGAttributeParser::Parse(SVGNumberType *number) {
  this->ParseWSToken();

  float s;
  if (this->ParseScalarToken(&s)) {
    *number = SVGNumberType(s);
    this->ParseSepToken();
    return true;
  }
  return false;
}

bool SVGAttributeParser::ParseInteger(SVGIntegerType *value) {
  this->ParseWSToken();
  this->ParseExpectedStringToken("+");

  SVGIntegerType i;
  if (this->ParseInt32Token(&i)) {
    *value = SVGNumberType(i);
    this->ParseSepToken();
    return true;
  }

  return false;
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeLength
template <>
bool SVGAttributeParser::Parse(SVGLength *length) {
  float s;
  SVGLength::Unit u = SVGLength::Unit::kNumber;

  if (this->ParseScalarToken(&s) && this->ParseLengthUnitToken(&u) ||
      this->ParseSepToken() || this->ParseEOSToken()) {
    *length = SVGLength{s, u};
    this->ParseSepToken();
    return true;
  }
  return false;
}

// https://www.w3.org/TR/SVG11/coords.html#ViewBoxAttribute
bool SVGAttributeParser::ParseViewBox(SVGViewBoxType *box) {
  float x, y, w, h;
  this->ParseWSToken();

  bool parsed_value = false;
  if (this->ParseScalarToken(&x) && this->ParseSepToken() &&
      this->ParseScalarToken(&y) && this->ParseSepToken() &&
      this->ParseScalarToken(&w) && this->ParseSepToken() &&
      this->ParseScalarToken(&h)) {
    *box = SVGViewBoxType{Rect::MakeXYWH(x, y, w, h)};
    parsed_value = true;

    this->ParseWSToken();
  }
  return parsed_value && this->ParseEOSToken();
}

template <typename Func, typename T>
bool SVGAttributeParser::ParseParenthesized(const char *prefix, Func func,
                                            T *result) {
  this->ParseWSToken();
  if (prefix && !this->ParseExpectedStringToken(prefix)) {
    return false;
  }
  this->ParseWSToken();
  if (!this->ParseExpectedStringToken("(")) {
    return false;
  }
  this->ParseWSToken();

  if (!func(result)) {
    return false;
  }
  this->ParseWSToken();

  return this->ParseExpectedStringToken(")");
}

// https://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint
template <>
bool SVGAttributeParser::Parse(SVGPaint *paint) {
  SVGColor c;
  bool parsed_value = false;
  if (this->Parse(&c)) {
    *paint = SVGPaint{c};
    parsed_value = true;
  } else if (this->ParseExpectedStringToken("none")) {
    *paint = SVGPaint{SVGPaint::Type::kNone};
    parsed_value = true;
  }

  return parsed_value && this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#StrokeLinecapProperty
template <>
bool SVGAttributeParser::Parse(SVGLineCap *cap) {
  static const struct {
    SVGLineCap type;
    const char *name;
  } gCapInfo[] = {
      {SVGLineCap::kButt, "butt"},
      {SVGLineCap::kRound, "round"},
      {SVGLineCap::kSquare, "square"},
  };

  bool parsed_value = false;
  for (auto const &i : gCapInfo) {
    if (this->ParseExpectedStringToken(i.name)) {
      *cap = SVGLineCap(i.type);
      parsed_value = true;
      break;
    }
  }

  return parsed_value && this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#StrokeLinejoinProperty
template <>
bool SVGAttributeParser::Parse(SVGLineJoin *join) {
  static const struct {
    SVGLineJoin::Type type;
    const char *name;
  } gJoinInfo[]{
      {SVGLineJoin::Type::kMiter, "miter"},
      {SVGLineJoin::Type::kRound, "round"},
      {SVGLineJoin::Type::kBevel, "bevel"},
      {SVGLineJoin::Type::kInherit, "inherit"},
  };

  bool parsed_value = false;
  for (auto const &i : gJoinInfo) {
    if (this->ParseExpectedStringToken(i.name)) {
      *join = SVGLineJoin(i.type);
      parsed_value = true;
      break;
    }
  }

  return parsed_value && this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/shapes.html#PolygonElementPointsAttribute
template <>
bool SVGAttributeParser::Parse(SVGPointsType *points) {
  std::vector<Point> pts;

  this->AdvanceWhile(is_ws);

  bool parsed_value = false;

  for (;;) {
    if (parsed_value && !this->ParseCommaWspToken()) {
      break;
    }

    float x, y;
    if (!this->ParseScalarToken(&x)) {
      break;
    }

    if (!this->ParseCommaWspToken() && !this->ParseEOSToken() &&
        *cur_pos != '-') {
      break;
    }

    if (!this->ParseScalarToken(&y)) {
      break;
    }

    pts.emplace_back(x, y, 0, 1.f);
    parsed_value = true;
  }

  if (parsed_value && this->ParseEOSToken()) {
    *points = pts;
    return true;
  }

  return false;
}

// https://www.w3.org/TR/SVG11/painting.html#VisibilityProperty
template <>
bool SVGAttributeParser::Parse(SVGVisibility *visibility) {
  // TODO implement this parser
  return false;
}

// https://www.w3.org/TR/SVG11/painting.html#StrokeDasharrayProperty
template <>
bool SVGAttributeParser::Parse(SVGDashArray *dash_array) {
  bool parsed_value = false;
  if (this->ParseExpectedStringToken("none")) {
    *dash_array = SVGDashArray(SVGDashArray::Type::kNone);
    parsed_value = true;
  } else if (this->ParseExpectedStringToken("inherit")) {
    *dash_array = SVGDashArray(SVGDashArray::Type::kInherit);
    parsed_value = true;
  } else {
    std::vector<SVGLength> dashes;
    for (;;) {
      SVGLength dash;
      if (!this->Parse(&dash)) {
        break;
      }

      dashes.emplace_back(dash);
      parsed_value = true;
    }

    if (parsed_value) {
      *dash_array = SVGDashArray(std::move(dashes));
    }
  }

  return parsed_value && this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeCoordinates
template <typename T>
bool SVGAttributeParser::ParseList(std::vector<T> *vals) {
  T v;
  for (;;) {
    if (!this->Parse(&v)) {
      break;
    }

    vals->emplace_back(v);
    this->ParseCommaWspToken();
  }

  return !vals->empty() && this->ParseEOSToken();
}

// https://www.w3.org/TR/SVG11/coords.html#PreserveAspectRatioAttribute
bool SVGAttributeParser::ParsePreserveAspectRatio(
    SVGPreserveAspectRatio *aspect_ratio) {
  static struct {
    const char *name;
    SVGPreserveAspectRatio::Align align;
  } gAlignMap[]{
      {"none", SVGPreserveAspectRatio::kNone},
      {"xMinYMin", SVGPreserveAspectRatio::kXMinYMin},
      {"xMidYMin", SVGPreserveAspectRatio::kXMidYMin},
      {"xMaxYMin", SVGPreserveAspectRatio::kXMaxYMin},
      {"xMinYMid", SVGPreserveAspectRatio::kXMinYMid},
      {"xMidYMid", SVGPreserveAspectRatio::kXMidYMid},
      {"xMidYMax", SVGPreserveAspectRatio::kXMidYMax},
      {"xMaxYMax", SVGPreserveAspectRatio::kXMaxYMax},
  };

  static struct {
    const char *name;
    SVGPreserveAspectRatio::Scale scale;
  } gScaleMap[] = {
      {"meet", SVGPreserveAspectRatio::kMeet},
      {"slice", SVGPreserveAspectRatio::kSlice},
  };

  bool parsed_value = false;
  // ignoring optional 'defer'
  this->ParseExpectedStringToken("defer");
  this->ParseWSToken();

  for (auto const &align : gAlignMap) {
    if (this->ParseExpectedStringToken(align.name)) {
      aspect_ratio->align = align.align;
      parsed_value = true;
      break;
    }
  }

  if (!parsed_value) {
    return false;
  }

  for (auto const &scale : gScaleMap) {
    if (this->ParseExpectedStringToken(scale.name)) {
      parsed_value = true;
      aspect_ratio->scale = scale.scale;
      break;
    }
  }

  return parsed_value && this->ParseEOSToken();
}

template <>
bool SVGAttributeParser::Parse(std::vector<SVGLength> *lengths) {
  return this->ParseList(lengths);
}

template <>
bool SVGAttributeParser::Parse(std::vector<SVGNumberType> *numbers) {
  return this->ParseList(numbers);
}

// https://www.w3.org/TR/SVG11/coords.html#TransformAttribute
template <>
bool SVGAttributeParser::Parse(SVGTransformType *t) {
  Matrix matrix = glm::identity<Matrix>();
  bool parsed = false;
  while (true) {
    Matrix m;
    if (!(this->ParseMatrixToken(&m) || this->ParseTranslateToken(&m) ||
          this->ParseScaleToken(&m) || this->ParseRotateToken(&m) ||
          this->ParseSkewXToken(&m) || this->ParseSkewYToken(&m))) {
      break;
    }

    matrix = m * matrix;
    parsed = true;
    this->ParseCommaWspToken();
  }

  this->ParseWSToken();
  if (!parsed || !this->ParseEOSToken()) {
    return false;
  }

  *t = SVGTransformType{matrix};
  return true;
}

}  // namespace skity
