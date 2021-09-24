#version 330 core

// these macros are same as GLVertex2::VertexType
#define VERTEX_TYPE_STROKE_AA 100.0
#define VERTEX_TYPE_NONE 0.0
#define VERTEX_TYPE_LINE_EDGE 1.0
#define VERTEX_TYPE_LINE_CAP 2.0
#define VERTEX_TYPE_LINE_BEVEL_JOIN 3.0
#define VERTEX_TYPE_LINE_ROUND_JOIN 4.0
#define VERTEX_TYPE_QUAD_IN 5.0
#define VERTEX_TYPE_QUAD_OUT 6.0
#define VERTEX_TYPE_LINE_ROUND 7.0
#define VERTEX_TYPE_STROKE_QUAD 8.0
#define VERTEX_TYPE_FILL_EDGE 9.0

// these macros are same as GLShader::UniversealShader::Type
#define USER_FRAGMENT_TYPE_STENCIL 0
#define USER_FRAGMENT_TYPE_AA_OUTLINE 1
#define USER_FRAGMENT_TYPE_PURE_COLOR 2

#define M_PI 3.1415926535897932384626433832795

uniform vec4 UserColor;
uniform ivec4 UserData1;
uniform vec4 UserData2;
uniform vec4 UserData3;

// [x, y]
in vec2 vPos;
// [mix, u, v]
in vec3 vPosInfo;

// final fragment color
out vec4 FragColor;

float cbrt(float x) {
  return sign(x) * pow(abs(x), 1.0 / 3.0);
}

vec2 EvalQuad(float t) {

  vec2 a = UserData2.zw;
  vec2 b = UserData3.xy;
  vec2 c = UserData3.zw;

  vec2 tt = vec2(t, t);
  return (a * tt + b) * tt + c;
}

float QuadDist(float t) {
  if (0.0 <= t && t <= 1.0) {
    vec2 q = EvalQuad(t) - vPos;
    float strokeWidth = UserData2.x;
    float theta = 0.2;
    if (strokeWidth < 5.0) {
      theta = 0.6;
    }
    strokeWidth = strokeWidth * strokeWidth / 4.0;

    float dist = dot(q, q);
    float t = dist / strokeWidth;
    if (t < (1 - theta)) {
      return 1.0;
    }

    return (1.0 - t) / theta;
  }

  return -1.0;
}

bool QuadCheck(float t) {
  if (0.0 <= t && t <= 1.0) {
    vec2 q = EvalQuad(t) - vPos;
    float strokeWidth = UserData2.x;
    strokeWidth = strokeWidth * strokeWidth / 4.0;
    if (dot(q, q) <= strokeWidth) {
      return false;
    }
  }

  return true;
}

// Determin UserInput color
// this can be:
//  1. pure color
//  2. gradient color
//  3. texture color
vec4 CalculateUserColor() {
  // TODO implement other type color
  return vec4(UserColor.rgb * UserColor.a, UserColor.a);
}

// line edge aa alpha
float CalculateLineEdgeAlpha(float y) {
  float feathe = min(y, 1.0);
  float theta = 0.9;
  if (UserData2.x < 5.0) {
    theta = 0.5;
  }
  if (abs(feathe) < theta) {
    return 1.0;
  }

  float alpha = 1.0 - abs(feathe);
  alpha /= (1.0 - theta);
  return alpha;
}
// line cap alpha
float CalculateLineCapAlpha() {
  float alpha = vPosInfo.y;
  return alpha * CalculateLineEdgeAlpha(vPosInfo.z);
}

float CalculateRoundCapAlpha() {
  vec2 center = vPosInfo.yz;
  float strokeRadius = UserData2.x * 0.5;
  float dist = length(vPos - center);

  return CalculateLineEdgeAlpha(dist / strokeRadius);
}

float CalculateStrokeQuadAlpha() {
  float p = vPosInfo.y;
  float q = vPosInfo.z;
  float offset = UserData2.y;

  float d = q * q / 4.0 + p * p * p / 27.0;
  if (d >= 0.0) {
    float c1 = -q / 2.0;
    float c2 = sqrt(d);

    float t = QuadDist(cbrt(c1 + c2) + cbrt(c1 - c2) + offset);
    if (t >= 0) {
      return t;
    }
  } else {
    float cos_3_theta = 3.0 * q * sqrt(-3.0 / p) / (2.0 * p);
    float theta = acos(cos_3_theta) / 3.0;
    float r = 2.0 * sqrt(-p / 3.0);

    float t = QuadDist(r * cos(theta) + offset);
    if (t >= 0.0) {
      return t;
    }

    t = QuadDist(r * cos(theta + 2.0 * M_PI / 3.0) + offset);
    if (t >= 0.0) {
      return t;
    }

    t = QuadDist(r * cos(theta + 4.0 * M_PI / 3.0) + offset);
    if (t >= 0.0) {
      return t;
    }
  }

  return 1.0;
}

float CalculateFillEdgeAlpha() {
  return vPosInfo.y;
}

// Determin fragment alpha
// this may generate alpha gradient if needs anti-alias
float CalculateFragmentAlpha(float posType) {
  if (posType == VERTEX_TYPE_LINE_EDGE) {
    return CalculateLineEdgeAlpha(vPosInfo.y);
  }
  if (posType == VERTEX_TYPE_LINE_CAP) {
    return CalculateLineCapAlpha();
  }
  if (posType == VERTEX_TYPE_LINE_ROUND) {
    return CalculateRoundCapAlpha();
  }
  if (posType == VERTEX_TYPE_STROKE_QUAD) {
    return CalculateStrokeQuadAlpha();
  }

  if (posType == VERTEX_TYPE_FILL_EDGE) {
    return CalculateFillEdgeAlpha();
  }
  return 1.0;
}

bool NeedDiscard(float posType) {
  if (posType == VERTEX_TYPE_LINE_ROUND) {
    vec2 center = vPosInfo.yz;
    float strokeRadius = UserData2.x * 0.5;
    if (length(vPos - center) > strokeRadius) {
      discard;
      return true;
    }
  }

  if (posType == VERTEX_TYPE_STROKE_QUAD) {
    float p = vPosInfo.y;
    float q = vPosInfo.z;
    float offset = UserData2.y;

    float d = q * q / 4.0 + p * p * p / 27.0;
    if (d >= 0.0) {
      float c1 = -q / 2.0;
      float c2 = sqrt(d);

      if (QuadCheck(cbrt(c1 + c2) + cbrt(c1 - c2) + offset)) {
        discard;
        return true;
      }
    } else {
      float cos_3_theta = 3.0 * q * sqrt(-3.0 / p) / (2.0 * p);
      float theta = acos(cos_3_theta) / 3.0;
      float r = 2.0 * sqrt(-p / 3.0);

      if (QuadCheck(r * cos(theta) + offset) &&
          QuadCheck(r * cos(theta + 2.0 * M_PI / 3.0) + offset) &&
          QuadCheck(r * cos(theta + 4.0 * M_PI / 3.0) + offset)) {
        discard;
        return true;
      }
    }
  }

  return false;
}

void main() {
  bool needAA = false;
  float posType = vPosInfo.x;
  if (posType > VERTEX_TYPE_STROKE_AA) {
    posType -= VERTEX_TYPE_STROKE_AA;
    needAA = true;
  } else if (posType != 0.0) {
    needAA = true;
  }

  if (NeedDiscard(posType)) {
    return;
  }

  vec4 finalColor = CalculateUserColor();
  float finalAlpha = 1.0;
  if (needAA) {
    float aaAlpha = CalculateFragmentAlpha(posType);
    if (UserData1.x != USER_FRAGMENT_TYPE_AA_OUTLINE) {
      // stencil need discard aa outline fragment
      if (aaAlpha < 1.0) {
        discard;
      }
    }

    finalAlpha = finalAlpha * aaAlpha;
  }

  FragColor = finalColor * finalAlpha;
}
