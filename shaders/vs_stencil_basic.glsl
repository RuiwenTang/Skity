#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aUV;
uniform mat4 mvp;

out vec3 TexCoord;
out vec3 vPos;

void main() {
  gl_Position = mvp * vec4(aPos.xy, 0.0, 1.0);
  TexCoord = aUV;
  vPos = aPos;
}