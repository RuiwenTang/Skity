#version 330 core

// these macros are same as GLVertex2::VertexType
#define VERTEX_TYPE_STROKE_AA 100.0
#define VERTEX_TYPE_NONE 0.0
#define VERTEX_TYPE_LINE_EDGE 1.0
#define VERTEX_TYPE_LINE_CAP 2.0
#define VERTEX_TYPE_LINE_BEVEL_JOIN 3.0
#define VERTEX_TYPE_LINE_ROUND_JOIN 4.0
#define VERTEX_TYPE_FILL_QUAD_IN 5.0
#define VERTEX_TYPE_QUAD_OUT 6.0
#define VERTEX_TYPE_LINE_ROUND 7.0
#define VERTEX_TYPE_STROKE_QUAD 8.0
#define VERTEX_TYPE_FILL_EDGE 9.0

// these macros are same as GLShader::UniversealShader::Type
#define USER_FRAGMENT_TYPE_STENCIL 0
#define USER_FRAGMENT_TYPE_AA_OUTLINE 1
#define USER_FRAGMENT_TYPE_PURE_COLOR 0x02
#define USER_FRAGMENT_TYPE_TEXTURE 0x04
#define USER_FRAGMENT_TYPE_GRADIENT_LINEAR 0x08
#define USER_FRAGMENT_TYPE_GRADIENT_RADIAL 0x10

#define M_PI 3.1415926535897932384626433832795
#define MAX_COLORS 32

uniform sampler2D UserTexture;
uniform vec4 UserColor;
uniform ivec4 UserData1;
uniform vec4 UserData2;
uniform vec4 UserData3;
uniform vec4 UserData4;
uniform vec4 GradientColors[MAX_COLORS];
uniform float GradientStops[MAX_COLORS];
uniform mat4 UserTransform;
uniform mat4 UserShaderMatrix;

// [x, y]
in vec2 vPos;
// [mix, u, v]
in vec3 vPosInfo;
// [offset, p1.x, p1.y]
in vec3 vQuadInfo1;
// [p2.x, p2.y, p3.x, p3.y]
in vec4 vQuadInfo2;

// final fragment color
out vec4 FragColor;

float cbrt(float x) { return sign(x) * pow(abs(x), 1.0 / 3.0); }

vec2 EvalQuad(float t) {
  vec2 a = vQuadInfo1.yz;
  vec2 b = vQuadInfo2.xy;
  vec2 c = vQuadInfo2.zw;

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

vec4 CalculateTextureColor() {
  // calculate UV first
  vec4 LeftTop =
      UserShaderMatrix * UserTransform * vec4(UserData4.xy, 0.0, 1.0);
  vec4 BottomRight =
      UserShaderMatrix * UserTransform * vec4(UserData4.zw, 0.0, 1.0);
  vec4 pos = UserTransform * vec4(vPos, 0.0, 1.0);

  float totalX = BottomRight.x - LeftTop.x;
  float totalY = BottomRight.y - LeftTop.y;

  float vX = (pos.x - LeftTop.x) / totalX;
  float vY = (pos.y - LeftTop.y) / totalY;
  vX = clamp(vX, 0.0, 1.0);
  vY = clamp(vY, 0.0, 1.0);

  return texture(UserTexture, vec2(vX, vY)) * UserColor.w;
}

vec4 LerpGradientColor(float dist) {
  if (dist >= 1.0) {
    dist = 1.0;
  }
  int ColorCount = UserData1.z;
  int StopCount = UserData1.w;

  int StartIndex = 0;
  int EndIndex = 1;
  float step = 1.0 / (ColorCount - 1);
  int i = 0;
  float Start, End;
  for (i = 0; i < ColorCount - 1; i++) {
    if (StopCount > 0) {
      Start = GradientStops[i];
      End = GradientStops[i + 1];
    } else {
      Start = step * i;
      End = step * (i + 1);
    }

    if (dist >= Start && dist <= End) {
      StartIndex = i;
      EndIndex = i + 1;
      break;
    }
  }

  if (i == ColorCount - 1 && ColorCount > 0) {
    return GradientColors[ColorCount - 1];
  }

  float total = (End - Start);
  float value = (dist - Start);
  float mixValue = 0.5;
  if (total > 0) {
    mixValue = value / total;
  }

  vec4 color;
  if (UserData1.y == 1) {
    color =
        mix(vec4(GradientColors[StartIndex].xyz * GradientColors[StartIndex].w,
                 GradientColors[StartIndex].w),
            vec4(GradientColors[EndIndex].xyz * GradientColors[EndIndex].w,
                 GradientColors[EndIndex].w),
            mixValue);
  } else {
    color = mix(GradientColors[StartIndex], GradientColors[EndIndex], mixValue);
  }

  return vec4(color.xyz * color.w, color.w);
}

vec4 CalculateLinearGradientColor() {
  vec4 StartPt =
      UserShaderMatrix * UserTransform * vec4(UserData4.xy, 0.0, 1.0);
  vec4 EndPt = UserShaderMatrix * UserTransform * vec4(UserData4.zw, 0.0, 1.0);
  vec4 CurrentPt = UserTransform * vec4(vPos, 0.0, 1.0);

  vec2 sc = CurrentPt.xy - StartPt.xy;
  vec2 se = EndPt.xy - StartPt.xy;

  if (sc.x * se.x + sc.y * se.y < 0) {
    return LerpGradientColor(0.0);
  }

  float mixValue = dot(sc, se) / length(se);
  float totalDist = length(se);

  return LerpGradientColor(mixValue / totalDist);
}

vec4 CalculateRadialGradient() {
  vec4 Center = UserShaderMatrix * UserTransform * vec4(UserData4.xy, 0.0, 1.0);
  vec4 CurrentPt = UserTransform * vec4(vPos, 0.0, 1.0);

  float mixValue = distance(Center.xy, CurrentPt.xy);

  return LerpGradientColor(mixValue / UserData4.z);
}

// Determin UserInput color
// this can be:
//  1. pure color
//  2. gradient color
//  3. texture color
vec4 CalculateUserColor() {
  // TODO implement other type color
  if ((UserData1.x & USER_FRAGMENT_TYPE_PURE_COLOR) != 0) {
    // just color
    return vec4(UserColor.rgb * UserColor.a, UserColor.a);
  } else if ((UserData1.x & USER_FRAGMENT_TYPE_TEXTURE) != 0) {
    // draw texture
    return CalculateTextureColor();
  } else if ((UserData1.x & USER_FRAGMENT_TYPE_GRADIENT_LINEAR) != 0) {
    return CalculateLinearGradientColor();
  } else if ((UserData1.x & USER_FRAGMENT_TYPE_GRADIENT_RADIAL) != 0) {
    return CalculateRadialGradient();
  }

  return vec4(0, 0, 0, 1);
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
  float offset = vQuadInfo1.x;

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

float CalculateFillEdgeAlpha() { return vPosInfo.y; }

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
    float offset = vQuadInfo1.x;

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

  if (posType == VERTEX_TYPE_FILL_QUAD_IN) {
    if (vPosInfo.y * vPosInfo.y - vPosInfo.z > 0) {
      discard;
      return true;
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
    if ((UserData1.x & USER_FRAGMENT_TYPE_AA_OUTLINE) == 0) {
      // stencil need discard aa outline fragment
      if (aaAlpha < 1.0) {
        discard;
      }
    }

    finalAlpha = finalAlpha * aaAlpha;
  }

  FragColor = finalColor * finalAlpha;
}
