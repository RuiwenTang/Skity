#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, location = 0) in vec2 inPosition;
layout(set = 0, location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;


void main() {
  gl_Position = vec4(inPosition, 0.0, 1.0);
  fragColor = inColor;
}