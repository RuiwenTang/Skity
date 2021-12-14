#version 450

// global pipeline info
layout(push_constant) uniform constants {
  mat4 mvp;
  int premulAlpha;
}
GlobalInfo;

layout(set = 0, binding = 0) uniform _Transform { mat4 uMatrix; }
TransformData;

// set 1 is fragment common
layout(set = 1, binding = 0) uniform _GlobalAlpha {
  // [global_alpha, stroke_width, TBD, TBD]
  vec4 info;
}
AlphaStroke;

layout(set = 2, binding = 1) uniform _UserColor { vec4 color; }
UserColor;

// vertex input
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vPosInfo;

layout(location = 0) out vec4 outColor;

void main() {
  if (GlobalInfo.premulAlpha == 0) {
    outColor =
        vec4(UserColor.color.rgb, UserColor.color.a * AlphaStroke.info.r);
  } else {
    vec4 color =
        vec4(UserColor.color.rgb * UserColor.color.a, UserColor.color.a);

    outColor = color * AlphaStroke.info.r;
  }
}