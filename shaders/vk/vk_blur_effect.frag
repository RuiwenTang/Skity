#version 450

#include <vk_common.glsl>
#include <vk_font_common.glsl>

#define PIPELINE_MODE_FBO_TEXTURE 5
#define PIPELINE_MODE_HORIZONTAL_BLUR 6
#define PIPELINE_MODE_VERTICAL_BLUR 7
#define PIPELINE_MODE_SOLID_BLUR 8
#define PIPELINE_MODE_OUTER_BLUR 9
#define PIPELINE_MODE_INNER_BLUR 10

// set 2 binding 0 is image bounds info
layout(set = 2, binding = 0) uniform _ImageBounds { vec4 info; }
ImageBounds;

// image texture is in set 2 binding 1
layout(set = 2, binding = 1) uniform sampler2D ImageTex;

// blur type
layout(set = 2, binding = 2) uniform _blur_type { ivec4 info; }
BlurType;

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

  vec4 image_color = texture(ImageTex, uv);
  if (GlobalInfo.premulAlpha == 0) {
    outColor = vec4(image_color.rgb, image_color.a * AlphaStroke.info.r);
  } else {
    vec4 color = vec4(image_color.rgb * image_color.a, image_color.a);

    outColor = color * AlphaStroke.info.r;
  }
}