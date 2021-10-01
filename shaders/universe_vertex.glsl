#version 330 core

// [x, y]
layout(location = 0) in vec2 aPos;
// dynamic info
layout(location = 1) in vec3 aPosInfo;

uniform mat4 mvp;
uniform mat4 UserTransform;

out vec2 vPos;
out vec3 vPosInfo;

void main() {
  vPos = aPos;
  vPosInfo = aPosInfo;
  gl_Position = mvp * UserTransform * vec4(aPos, 0.0, 1.0);
}