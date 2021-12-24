
#define M_PI 3.1415926535897932384626433832795
// Fixme to solve uniform array length
#define MAX_COLORS 32

#define PIPELINE_MODE_STENCIL 0
#define PIPELINE_MODE_UNIFORM_COLOR 1
#define PIPELINE_MODE_IMAGE_TEXTURE 2
#define PIPELINE_MODE_LINEAR_GRADIENT 3
#define PIPELINE_MODE_RADIAL_GRADIENT 4

#define VERTEX_TYPE_LINE_NORMAL 1
#define VERTEX_TYPE_CIRCLE 2
#define VERTEX_TYPE_QUAD_IN 3
#define VERTEX_TYPE_QUAD_OUT 4
#define VERTEX_TYPE_TEXT 5

// image texture
uniform sampler2D UserTexture;
// font texture
uniform sampler2D FontTexture;

// transform matrix
uniform mat4 UserTransform;
// global alpha
uniform float GlobalAlpha;
// uniform color set from Paint
uniform vec4 UserColor;
// stroke width or circle radius
uniform float StrokeWidth;
// color type
uniform int ColorType;
// [color_count, pos_count]
uniform ivec2 GradientCounts;
// [p1.x, p1.y, p2.x, p2.y]
uniform vec4 GradientBounds;

// gradient color and stops
uniform vec4 GradientColors[MAX_COLORS];
uniform float GradientStops[MAX_COLORS];

// [x, y]
in vec2 vPos;
// [mix, u, v]
in vec3 vPosInfo;

out vec4 FragColor;

vec4 lerp_color(float current) {
  if (current > 1.0) {
    current = 1.0;
  }

  int colorCount = GradientCounts[0];
  int stopCount = GradientCounts[1];
  int premulAlpha = 0;

  int startIndex = 0;
  int endIndex = 1;

  float step = 1.0 / (colorCount - 1);
  int i = 0;
  float start, end;
  for (i = 0; i < colorCount - 1; i++) {
    if (stopCount > 0) {
      start = GradientStops[i];
      end = GradientStops[i + 1];
    } else {
      start = step * i;
      end = step * (i + 1);
    }

    if (current >= start && current <= end) {
      startIndex = i;
      endIndex = i + 1;
      break;
    }
  }

  if (i == colorCount - 1 && colorCount > 0) {
    return GradientColors[colorCount - 1];
  }

  float total = (end - start);
  float value = (current - start);

  float mixValue = 0.5;
  if (total > 0) {
    mixValue = value / total;
  }

  vec4 color;
  if (premulAlpha == 1) {
    color =
        mix(vec4(GradientColors[startIndex].xyz * GradientColors[startIndex].w,
                 GradientColors[startIndex].w),
            vec4(GradientColors[endIndex].xyz * GradientColors[endIndex].w,
                 GradientColors[endIndex].w),
            mixValue);
  } else {
    color = mix(GradientColors[startIndex], GradientColors[endIndex], mixValue);
  }

  return color;
}

vec4 calculate_radial_color() {
  vec4 mappedCenter = UserTransform * vec4(GradientBounds.xy, 0.0, 1.0);
  vec4 currentPoint = UserTransform * vec4(vPos, 0.0, 1.0);

  float mixValue = distance(mappedCenter.xy, currentPoint.xy);
  return lerp_color(mixValue / GradientBounds.z);
}

vec4 calculate_linear_color() {
  vec4 startPointMaped = UserTransform * vec4(GradientBounds.xy, 0.0, 1.0);
  vec4 endPointMapped = UserTransform * vec4(GradientBounds.zw, 0.0, 1.0);
  vec4 currentPoint = UserTransform * vec4(vPos, 0.0, 1.0);

  vec2 sc = vec2(currentPoint.x - startPointMaped.x,
                 currentPoint.y - startPointMaped.y);
  vec2 se = (endPointMapped - startPointMaped).xy;

  if (sc.x * se.x + sc.y * se.y < 0) {
    return lerp_color(0.0);
  }

  float mixValue = dot(sc, se) / length(se);
  float totalDist = length(se);
  return lerp_color(mixValue / totalDist);
}

vec4 calculate_gradient_color() {
  if (ColorType == PIPELINE_MODE_LINEAR_GRADIENT) {
    return calculate_linear_color();
  } else if (ColorType == PIPELINE_MODE_RADIAL_GRADIENT) {
    return calculate_radial_color();
  } else {
    return vec4(1.0, 0.0, 0.0, 1.0);
  }
}

vec2 calculate_uv() {
  vec4 mappedLT = vec4(GradientBounds.xy, 0.0, 1.0);
  vec4 mappedBR = vec4(GradientBounds.zw, 0.0, 1.0);

  vec4 mappedPos = vec4(vPos.xy, 0.0, 1.0);

  float totalX = mappedBR.x - mappedLT.x;
  float totalY = mappedBR.y - mappedLT.y;

  float vX = (mappedPos.x - mappedLT.x) / totalX;
  float vY = (mappedPos.y - mappedLT.y) / totalY;
  return vec2(vX, vY);
}

void main() {
  int vertex_type = int(vPosInfo.x);

  if (vertex_type == VERTEX_TYPE_CIRCLE) {
    float r = length(vPos - vPosInfo.yz);
    if (r > StrokeWidth / 2.0) {
      discard;
    }
  } else if (vertex_type == VERTEX_TYPE_QUAD_IN ||
             vertex_type == VERTEX_TYPE_QUAD_OUT) {
    float f_x = vPosInfo.y * vPosInfo.y - vPosInfo.z;
    if (vertex_type == VERTEX_TYPE_QUAD_IN && f_x > 0.0) {
      discard;
    }

    if (vertex_type == VERTEX_TYPE_QUAD_OUT && f_x < 0.0) {
      discard;
    }
  }

  if (ColorType == PIPELINE_MODE_STENCIL) {
    FragColor = vec4(0, 0, 0, 0);
  } else if (ColorType == PIPELINE_MODE_UNIFORM_COLOR) {
    FragColor = vec4(UserColor.xyz * UserColor.w, UserColor.w) * GlobalAlpha;
  } else if (ColorType == PIPELINE_MODE_IMAGE_TEXTURE) {
    // Texture sampler
    vec2 uv = calculate_uv();

    uv.x = clamp(uv.x, 0.0, 1.0);

    uv.y = clamp(uv.y, 0.0, 1.0);
    FragColor = texture(UserTexture, uv) * GlobalAlpha;
  } else {
    vec4 g_color = calculate_gradient_color();
    FragColor = vec4(g_color.xyz * g_color.w, g_color.w) * GlobalAlpha;
  }

  if (vertex_type == VERTEX_TYPE_TEXT) {
    float r = texture(FontTexture, vec2(vPosInfo.y, vPosInfo.z)).r;
    FragColor = FragColor * r;
  }
}