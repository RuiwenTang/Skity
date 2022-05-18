#version 450

#include <vk_common.glsl>
#include <vk_font_common.glsl>

// Fixme to solve uniform array length
#define MAX_COLORS 32

layout(set = 2, binding = 0) uniform _GradientInfo {
  ivec4 count;
  vec4 bounds;
  vec4 GradientColors[MAX_COLORS];
  vec4 GradientStops[MAX_COLORS / 4];
}
GradientInfo;

float get_gradient_stop(int index) {
  int i = index / 4;
  int j = index % 4;

  vec4 num = GradientInfo.GradientStops[i];
  float stop = num[j];

  return stop;
}

vec4 lerp_color(float current) {
  if (current > 1.0) {
    current = 1.0;
  }

  int colorCount = GradientInfo.count[0];
  int stopCount = GradientInfo.count[1];
  int premulAlpha = 0;

  int startIndex = 0;
  int endIndex = 1;

  float step = 1.0 / (colorCount - 1);
  int i = 0;
  float start, end;
  for (i = 0; i < colorCount - 1; i++) {
    if (stopCount > 0) {
      start = get_gradient_stop(i);
      end = get_gradient_stop(i + 1);
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
    return GradientInfo.GradientColors[colorCount - 1];
  }

  float total = (end - start);
  float value = (current - start);

  float mixValue = 0.5;
  if (total > 0) {
    mixValue = value / total;
  }

  vec4 color;
  if (premulAlpha == 1) {
    color = mix(vec4(GradientInfo.GradientColors[startIndex].xyz *
                         GradientInfo.GradientColors[startIndex].w,
                     GradientInfo.GradientColors[startIndex].w),
                vec4(GradientInfo.GradientColors[endIndex].xyz *
                         GradientInfo.GradientColors[endIndex].w,
                     GradientInfo.GradientColors[endIndex].w),
                mixValue);
  } else {
    color = mix(GradientInfo.GradientColors[startIndex],
                GradientInfo.GradientColors[endIndex], mixValue);
  }

  return color;
}

vec4 calculate_radial_color() {
  vec4 mappedCenter =
      TransformData.uMatrix * vec4(GradientInfo.bounds.xy, 0.0, 1.0);
  vec4 currentPoint = TransformData.uMatrix * vec4(vPos, 0.0, 1.0);

  float mixValue = distance(mappedCenter.xy, currentPoint.xy);
  return lerp_color(mixValue / GradientInfo.bounds.z);
}

vec4 calculate_linear_color() {
  vec4 startPointMaped =
      TransformData.uMatrix * vec4(GradientInfo.bounds.xy, 0.0, 1.0);
  vec4 endPointMapped =
      TransformData.uMatrix * vec4(GradientInfo.bounds.zw, 0.0, 1.0);
  vec4 currentPoint = TransformData.uMatrix * vec4(vPos, 0.0, 1.0);

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
  int ColorType = GradientInfo.count.z;
  if (ColorType == PIPELINE_MODE_LINEAR_GRADIENT) {
    return calculate_linear_color();
  } else if (ColorType == PIPELINE_MODE_RADIAL_GRADIENT) {
    return calculate_radial_color();
  } else {
    return vec4(1.0, 0.0, 0.0, 1.0);
  }
}

void main() {
  vec4 gadient_color = calculate_gradient_color();
  if (GlobalInfo.premulAlpha == 0) {
    outColor = vec4(gadient_color.rgb, gadient_color.a * AlphaStroke.info.r);
  } else {
    vec4 color = vec4(gadient_color.rgb * gadient_color.a, gadient_color.a);

    outColor = color * AlphaStroke.info.r;
  }

  calculate_font_discard();
}