#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aUV;
uniform mat4 mvp;

out vec3 TexCoord;

void main() {
  gl_Position = mvp * vec4(aPos, 0.0, 1.0);
  TexCoord = aUV;
}