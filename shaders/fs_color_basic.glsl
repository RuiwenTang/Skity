
#version 330 core

out vec4 FragColor;

uniform vec4 user_color;

in float vAlpha;

void main() {
  vec4 vColor = user_color;
  vColor = vec4(vColor.xyz * vColor.w, vColor.w);

  if (vAlpha < 1.0) {
    vColor = vColor * vAlpha;
  }

  FragColor = vColor;
}