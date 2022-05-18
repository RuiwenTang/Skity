#version 450

#include <vk_common.glsl>

void main() {
  // stencil pipeline has no color output
  outColor = vec4(1, 1, 1, 1);
}