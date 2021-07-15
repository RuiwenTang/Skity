#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 mvp;

out float vAlpha;

void main() {
  vAlpha = aPos.z;
  gl_Position = mvp * vec4(aPos.xy, 0.0, 1.0);
}