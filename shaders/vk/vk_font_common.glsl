

// set 3 is color fragment common for font
layout(set = 3, binding = 0) uniform sampler2D FontTex;

void calculate_font_discard() {
  int vertex_type = int(vPosInfo.x);

  if (vertex_type == VERTEX_TYPE_TEXT) {
    float r = texture(FontTex, vec2(vPosInfo.y, vPosInfo.z)).r;
    outColor = outColor * r;
  }
}