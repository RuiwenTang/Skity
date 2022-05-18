
#define M_PI 3.1415926535897932384626433832795
// Fixme to solve uniform array length
#define MAX_COLORS 32

#define PIPELINE_MODE_STENCIL 0
#define PIPELINE_MODE_UNIFORM_COLOR 1
#define PIPELINE_MODE_IMAGE_TEXTURE 2
#define PIPELINE_MODE_LINEAR_GRADIENT 3
#define PIPELINE_MODE_RADIAL_GRADIENT 4
#define PIPELINE_MODE_FBO_TEXTURE 5
#define PIPELINE_MODE_HORIZONTAL_BLUR 6
#define PIPELINE_MODE_VERTICAL_BLUR 7
#define PIPELINE_MODE_SOLID_BLUR 8
#define PIPELINE_MODE_OUTER_BLUR 9
#define PIPELINE_MODE_INNER_BLUR 10

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

  float step = 1.0 / float(colorCount - 1);
  int i = 0;
  float start, end;
  for (i = 0; i < colorCount - 1; i++) {
    if (stopCount > 0) {
      start = GradientStops[i];
      end = GradientStops[i + 1];
    } else {
      start = step * float(i);
      end = step * float(i + 1);
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
  if (total > 0.0) {
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

  if (sc.x * se.x + sc.y * se.y < 0.0) {
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

// all blur calculation is based on
// https://www.geeks3d.com/20100909/shader-library-gaussian-blur-post-processing-filter-in-glsl/
float calculate_blur_norm() {
  float sigma = StrokeWidth + 2.0;
  return 1.0 / (sqrt(2.0 * M_PI) * sigma);
}

float calculate_blur_coffe(float norm, float step) {
  // blur mode kernelSize is passed through StrokeWidth
  float sigma = StrokeWidth + 2.0;
  return norm * exp(-1.5 * step * step / (sigma * sigma));
}

vec4 calculate_blur(vec2 uv, vec2 dir, vec2 step_vec) {
  float norm = calculate_blur_norm();

  float total = norm;

  vec4 acc = texture(UserTexture, uv) * norm;

  int kernel_size = int(StrokeWidth);
  for (int i = 1; i <= kernel_size; i++) {
    float coffe = calculate_blur_coffe(norm, float(i));
    float f_i = float(i);

    acc += texture(UserTexture, uv - f_i * step_vec * dir) * coffe;
    acc += texture(UserTexture, uv + f_i * step_vec * dir) * coffe;

    total += 2.0 * coffe;
  }

  acc = acc / total;
  return acc;
}

vec4 calculate_vertical_blur(vec2 uv) {
  vec2 step_vec = vec2(1.0 / (GradientBounds.z - GradientBounds.x),
                       1.0 / (GradientBounds.w - GradientBounds.y));
  vec2 dir = vec2(0.0, 1.0);

  return calculate_blur(uv, dir, step_vec);
}

vec4 calculate_horizontal_blur(vec2 uv) {
  vec2 step_vec = vec2(1.0 / (GradientBounds.z - GradientBounds.x),
                       1.0 / (GradientBounds.w - GradientBounds.y));
  vec2 dir = vec2(1.0, 0.0);

  return calculate_blur(uv, dir, step_vec);
}

vec4 calculate_solid_blur(vec2 uv) {
  uv = vec2(uv.x, 1.0 - uv.y);

  vec4 raw_color = texture(FontTexture, uv);
  if (raw_color.a > 0.0) {
    return raw_color;
  } else {
    return texture(UserTexture, uv);
  }
}

vec4 calculate_outer_blur(vec2 uv) {
  uv = vec2(uv.x, 1.0 - uv.y);
  vec4 raw_color = texture(FontTexture, uv);
  vec4 blur_color = texture(UserTexture, uv);

  if (raw_color.a > 0.0 && raw_color.a >= blur_color.a) {
    return vec4(0.0, 0.0, 0.0, 0.0);
  } else {
    return texture(UserTexture, uv);
  }
}

vec4 calculate_inner_blur(vec2 uv) {
  uv = vec2(uv.x, 1.0 - uv.y);

  vec4 raw_color = texture(FontTexture, uv);
  vec4 blur_color = texture(UserTexture, uv);

  if (raw_color.a > 0.0) {
    return blur_color * (raw_color.a - blur_color.a);
  } else {
    return vec4(0.0, 0.0, 0.0, 0.0);
  }
}

void main() {
  int vertex_type = int(vPosInfo.x);

  if (ColorType == PIPELINE_MODE_STENCIL) {
    FragColor = vec4(0, 0, 0, 0);
  } else if (ColorType == PIPELINE_MODE_UNIFORM_COLOR) {
    FragColor = vec4(UserColor.xyz * UserColor.w, UserColor.w) * GlobalAlpha;
  } else if (ColorType == PIPELINE_MODE_IMAGE_TEXTURE ||
             (ColorType >= PIPELINE_MODE_FBO_TEXTURE &&
              ColorType <= PIPELINE_MODE_INNER_BLUR)) {
    // Texture sampler
    vec2 uv = calculate_uv();

    uv.x = clamp(uv.x, 0.0, 1.0);

    uv.y = clamp(uv.y, 0.0, 1.0);

    // TODO this code need optimize
    if (ColorType == PIPELINE_MODE_HORIZONTAL_BLUR) {
      FragColor = calculate_horizontal_blur(uv);
      return;
    } else if (ColorType == PIPELINE_MODE_VERTICAL_BLUR) {
      FragColor = calculate_vertical_blur(uv);
      return;
    } else if (ColorType == PIPELINE_MODE_SOLID_BLUR) {
      FragColor = calculate_solid_blur(uv);
      return;
    } else if (ColorType == PIPELINE_MODE_OUTER_BLUR) {
      FragColor = calculate_outer_blur(uv);
      return;
    } else if (ColorType == PIPELINE_MODE_INNER_BLUR) {
      FragColor = calculate_inner_blur(uv);
      return;
    }

    if (ColorType == PIPELINE_MODE_FBO_TEXTURE) {
      uv.y = 1.0 - uv.y;
    }
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