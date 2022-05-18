#version 450

#include <vk_common.glsl>
#include <vk_font_common.glsl>

layout(set = 2, binding = 0) uniform _UserColor { vec4 color; }
UserColor;

void main() {
  if (GlobalInfo.premulAlpha == 0) {
    outColor =
        vec4(UserColor.color.rgb, UserColor.color.a * AlphaStroke.info.r);
  } else {
    vec4 color =
        vec4(UserColor.color.rgb * UserColor.color.a, UserColor.color.a);

    outColor = color * AlphaStroke.info.r;
  }

  calculate_font_discard();
}