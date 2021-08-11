#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 mvp;

out float vAlpha;
out vec2 vPos;

void main() {
  vec4 pos = mvp * vec4(aPos.xy, 0.0, 1.0);
  gl_Position = pos;

  vAlpha = aPos.z;
  vPos = aPos.xy;
}