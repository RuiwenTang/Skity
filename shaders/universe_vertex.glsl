#version 330 core

// [x, y]
layout(location = 0) in vec2 aPos;
// dynamic info
layout(location = 1) in vec3 aPosInfo;
// quad info 1 [offset, p1.x, p1.y]
layout(location = 2) in vec3 aQuadInfo1;
// quad info 2 [p2.x, p2.y, p3.x, p3.y]
layout(location = 3) in vec4 aQuadInfo2;

uniform mat4 mvp;
uniform mat4 UserTransform;

out vec2 vPos;
out vec3 vPosInfo;
out vec3 vQuadInfo1;
out vec4 vQuadInfo2;

void main() {
  vPos = aPos;
  vPosInfo = aPosInfo;
  vQuadInfo1 = aQuadInfo1;
  vQuadInfo2 = aQuadInfo2;
  gl_Position = mvp * UserTransform * vec4(aPos, 0.0, 1.0);
}