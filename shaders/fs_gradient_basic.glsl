#version 330 core

uniform mat4 localMatrix;

#define MAX_COLORS 32
#define GRADIENT_TYPE_LINEAR 2
#define GRADIENT_TYPE_RADIAL 3
#define GRADIENT_TYPE_SWEEP 4

uniform vec2 points[2];
uniform vec2 radius[2];
uniform int colorCount;
uniform int gradientType;
uniform int stopCount;
uniform vec4 colors[MAX_COLORS];
uniform float colorStops[MAX_COLORS];

in float vAlpha;
in vec2 vPos;

out vec4 FragColor;

vec4 lerp_color(float current) {
  if (current > 1.0) {
    current = 1.0;
  }

  int startIndex = 0;
  int endIndex = 1;
  float step = 1.0 / (colorCount - 1);
  int i = 0;
  float start, end;
  for (i = 0; i < colorCount - 1; i++) {
    if (stopCount > 0) {
      start = colorStops[i];
      end = colorStops[i + 1];
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

  if (i == colorCount - 1) {
    return colors[colorCount - 1];
  }

  float total = (end - start);
  float value = (current - start);

  float mixValue = 0.5;
  if (total > 0) {
    mixValue = value / total;
  }

  vec4 color = mix(colors[startIndex], colors[endIndex], mixValue);

  return color;
}

vec4 calculate_linear_color() {
  vec4 startPointMaped = localMatrix * vec4(points[0], 0.0, 1.0);
  vec4 endPointMapped = localMatrix * vec4(points[1], 0.0, 1.0);
  vec4 currentPoint = localMatrix * vec4(vPos, 0.0, 1.0);

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

vec4 calculate_radial_color() { return vec4(1.0, 1.0, 1.0, 1.0); }

vec4 calculate_color() {
  if (gradientType == GRADIENT_TYPE_LINEAR) {
    return calculate_linear_color();
  } else if (gradientType == GRADIENT_TYPE_RADIAL) {
    return calculate_radial_color();
  } else {
    return vec4(1.0, 0.0, 0.0, 1.0);
  }
}

void main() {
  vec4 vColor = calculate_color();

  if (vAlpha < 1.0) {
    vColor = vColor * vAlpha;
  }

  FragColor = vColor;
}
