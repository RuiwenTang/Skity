#version 450

// [x, y]
layout(set = 0, location = 0) in vec2 aPos;
layout(set = 0, location = 1) in vec3 aPosInfo;

layout(location = 0) out vec2 vPos;
layout(location = 1) out vec3 vPosInfo;

// set 0 is transform data, this is common for all pipeline shader
// available in both vertex and fragment shader stages
layout(set = 0, binding = 0) uniform _Transform { mat4 uMatrix; }
TransformData;

void main() {
  vPos = aPos;
  vPosInfo = aPosInfo;
  gl_Position = vec4(aPos, 0.0, 1.0);
}