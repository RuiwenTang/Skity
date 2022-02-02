#version 450

#include <vk_common.glsl>
#include <vk_font_common.glsl>

#define M_PI 3.1415926535897932384626433832795

#define PIPELINE_MODE_FBO_TEXTURE 5
#define PIPELINE_MODE_HORIZONTAL_BLUR 6
#define PIPELINE_MODE_VERTICAL_BLUR 7
#define PIPELINE_MODE_SOLID_BLUR 8
#define PIPELINE_MODE_OUTER_BLUR 9
#define PIPELINE_MODE_INNER_BLUR 10

// set 2 binding 0 is image bounds info
layout(set = 2, binding = 0) uniform _ImageBounds { vec4 info; }
ImageBounds;

// blur type
layout(set = 2, binding = 1) uniform _blur_type { ivec4 info; }
BlurType;

// image texture is in set 2 binding 1
layout(set = 2, binding = 2) uniform sampler2D ImageTex;

vec4 calculate_solid_blur(vec2 uv) {
  vec4 raw_color = texture(FontTex, uv);
  if (raw_color.a > 0.0) {
    return raw_color;
  } else {
    return texture(ImageTex, uv);
  }
}

vec4 calculate_outer_blur(vec2 uv) {
  vec4 raw_color = texture(FontTex, uv);

  if (raw_color.a > 0.0) {
    return vec4(0.0, 0.0, 0.0, 0.0);
  } else {
    return texture(ImageTex, uv);
  }
}

vec4 calculate_inner_blur(vec2 uv) {
  vec4 raw_color = texture(FontTex, uv);
  vec4 blur_color = texture(ImageTex, uv);

  if (raw_color.a > 0.0) {
    return blur_color * (raw_color.a - blur_color.a);
  } else {
    return vec4(0.0, 0.0, 0.0, 0.0);
  }
}

vec2 calculate_uv() {
  vec4 mappedLT = vec4(ImageBounds.info.xy, 0.0, 1.0);
  vec4 mappedBR = vec4(ImageBounds.info.zw, 0.0, 1.0);

  vec4 mappedPos = vec4(vPos.xy, 0.0, 1.0);

  float totalX = mappedBR.x - mappedLT.x;
  float totalY = mappedBR.y - mappedLT.y;

  float vX = (mappedPos.x - mappedLT.x) / totalX;
  float vY = (mappedPos.y - mappedLT.y) / totalY;
  return vec2(vX, vY);
}

void main() {
  vec2 uv = calculate_uv();

  uv.x = clamp(uv.x, 0.0, 1.0);

  uv.y = clamp(uv.y, 0.0, 1.0);

  uv.y = 1.0 - uv.y;

  if (BlurType.info.x == PIPELINE_MODE_SOLID_BLUR) {
    outColor = calculate_solid_blur(uv);
  } else if (BlurType.info.x == PIPELINE_MODE_OUTER_BLUR) {
    outColor = calculate_outer_blur(uv);
  } else if (BlurType.info.x == PIPELINE_MODE_INNER_BLUR) {
    outColor = calculate_inner_blur(uv);
  } else {
    outColor = texture(ImageTex, uv);
  }
}