#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_image_load_store : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in float vertType;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConsts { vec4 screenInfo; }
pushConsts;

layout(set = 0, binding = 0, rgba8) uniform image2D pathImage;

void main() {
  if (vertType > 0.0) {
    imageStore(pathImage, ivec2(1, 1), vec4(1.0, 1.0, 1.0, 1.0));
    outColor = fragColor;
  } else {
    vec4 c = imageLoad(pathImage, ivec2(1, 1));
    outColor = c;
  }
}