#version 330 core

#define M_PI 3.1415926535897932384626433832795
// Fixme to solve uniform array length
#define MAX_COLORS 32

#define PIPELINE_MODE_STENCIL 0
#define PIPELINE_MODE_UNIFORM_COLOR 1
#define PIPELINE_MODE_IMAGE_TEXTURE 2
#define PIPELINE_MODE_LINEAR_GRADIENT 3
#define PIPELINE_MODE_RADIAL_GRADIENT 4

#define VERTEX_TYPE_LINE_NORMAL 1
#define VERTEX_TYPE_CIRCLE 2
#define VERTEX_TYPE_QUAD_IN 3
#define VERTEX_TYPE_QUAD_OUT 4

// image texture
uniform sampler2D UserTexture;
// font texture
uniform sampler2D FontTexture;

// uniform color set from Paint
uniform vec4 UserColor;
// stroke width or circle radius
uniform float StrokeWidth;
// color type
uniform int ColorType;
// [color_count, pos_count]
uniform ivec2 GradientCounts;
// [p1.x, p1.y, p2.x, p2.y]
uniform vec4 GradientBounds;

// gradient color and stops
uniform vec4 GradientColors[MAX_COLORS];
uniform float GradientStops[MAX_COLORS];

// [x, y]
in vec2 vPos;
// [mix, u, v]
in vec3 vPosInfo;

out vec4 FragColor;

void main() {
  int vertex_type = int(vPosInfo.x);

  if (vertex_type == VERTEX_TYPE_CIRCLE) {
    float r = length(vPos - vPosInfo.yz);
    if (r > StrokeWidth / 2.0) {
      discard;
    }
  }

  if (ColorType == PIPELINE_MODE_STENCIL) {
    FragColor = vec4(0, 0, 0, 0);
  } else {
    FragColor = UserColor;
  }
}