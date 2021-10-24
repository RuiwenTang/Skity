#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in float vertType;
layout(location = 0) out vec4 outColor;

void main() {
  if (vertType < 0.0) {
    discard;
  }
  outColor = fragColor;
}