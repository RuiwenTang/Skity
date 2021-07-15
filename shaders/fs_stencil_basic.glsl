#version 330 core

in vec3 TexCoord;
in vec3 vPos;

out vec4 FragColor;

//
#define VERTEX_TYPE_NORMAL 0.0
#define VERTEX_TYPE_QUAD 1.0
#define VERTEX_TYPE_QUAD_OFF 2.0
#define VERTEX_TYPE_RADIUS 3.0

uniform float stroke_radius;

void main() {
  if (TexCoord.x != VERTEX_TYPE_NORMAL) {
    if (TexCoord.x == VERTEX_TYPE_RADIUS) {
      float dist = (vPos.x - TexCoord.y) * (vPos.x - TexCoord.y) +
                   (vPos.y - TexCoord.z) * (vPos.y - TexCoord.z);
      if (dist > stroke_radius * stroke_radius) {
        discard;
      }
    } else {
      float f_x = TexCoord.y * TexCoord.y - TexCoord.z;
      if (TexCoord.x == VERTEX_TYPE_QUAD && f_x > 0) {
        discard;
      }
      if (TexCoord.x == VERTEX_TYPE_QUAD_OFF && f_x < 0) {
        discard;
      }
    }
  }

  FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}