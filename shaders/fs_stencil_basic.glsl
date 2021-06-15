#version 330 core

in vec3 TexCoord;

out vec4 FragColor;

//
#define VERTEX_TYPE_NORMAL 0.0
#define VERTEX_TYPE_QUAD 1.0
#define VERTEX_TYPE_QUAD_OFF -1.0

void main() {
  if (TexCoord.x != VERTEX_TYPE_NORMAL) {
    float f_x = TexCoord.y * TexCoord.y - TexCoord.z;
    if (TexCoord.x == VERTEX_TYPE_QUAD && f_x > 0) {
      discard;
    }
    if (TexCoord.x == VERTEX_TYPE_QUAD_OFF && f_x < 0) {
      discard;
    }
  }

  FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}